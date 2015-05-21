#include <stdio.h>
#include <math.h>

#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"


#include "gre.h"
#include "escontext.h"
#include "repeating_task.h"

#include "util.h"


#define KEYSIZE 18
#define MAX_NAME_LENGTH 30

typedef struct {
  List attributes; // :: [Attribute*]
  List textures; // :: [Uniform*]
} SoData;

typedef struct {
  // gles resource handles
  List listPrograms; // :: [GLuint]
  List listVShaders; // :: [GLuint]
  List listFShaders; // :: [GLuint]
  List listBuffers; // :: [GLuint]
  List listTextures;// :: [GLuint]

  // mapp from application resource to gl resource
  HashMap mapBuffer2Handle; // :: Attribute* -> GLuint
  HashMap mapShaderShader2Handle; // :: Shader,Shader* -> GLuint
  HashMap mapVShader2Handle; //const char*-> GLuint
  HashMap mapFShader2Handle; //const char*-> GLuint
  HashMap mapTexture2Handle; // :: Uniform* -> GLuint

  // reference counts
  HashMap countBufferUsers; // :: Attribute* -> size_t
  HashMap countVShaderUsers; // ::const char*-> size_t
  HashMap countFShaderUsers; // ::const char*-> size_t
  HashMap countProgramUsers; // :: Shader,Shader -> size_t
  HashMap countTextureUsers; // :: Uniform* -> size_t

  // internal data structure for scene object
  HashMap mapSo2SoData; // :: SceneObject* -> soData*

  List removeSceneObjects; // [SceneObject*]
  List addSceneObjects;

  List stageGbuffer;
  List stageLight;
  List stageGeometry;
  List stageParticle;  
  List stageOverlay;

  GLuint fbos[2];
  GLuint rbos[1];
  GLuint tbos[2];

} SceneData;

HashMap mapSceneData;
HashMap staticPrograms;
HashMap staticShaders;


static const char* vGShader =
  "attribute vec4 a_position;\n"
  "attribute vec3 a_normal;\n"
  "uniform mat4 u_proj;\n"
  "uniform mat4 u_view;\n"
  "uniform mat4 u_world;\n"
  "uniform float u_farplane;\n"
  "varying vec3 v_normal;\n"  
  "varying float v_depth;\n"
  "\n"
  "void main() {\n"
  "  vec4 viewpos = u_view * u_world * a_position;\n"
  "  gl_Position = u_proj * viewpos;\n"
  "  v_depth = viewpos.z / u_farplane;\n" // divide by far plane, so depth is in [0.0; 1.0]
  "  v_normal = (u_view * u_world * vec4(a_normal, 0.0)).xyz;\n"
  "}\n";

static const char* fGShader =
  "varying vec3 v_normal;\n"
  "varying float v_depth;\n"  
  "\n"
  "vec2 encodeNormal (in vec3 normal) {\n"   
  "  return normal.xy * 0.5 + 0.5;\n"
  "}\n"
  "\n"
  "vec2 packFloat(in float value) {\n"
  "  return vec2(floor(value * 256.0) / 256.0, fract(value * 256.0));\n"
  "}\n"
  "\n"
  "void main() {\n"  
  "  gl_FragColor = vec4(encodeNormal(normalize(v_normal)), packFloat(v_depth));\n"  
  "}\n";


///////////////////////////////////////////////////////////////
///////////    UTILITY
///////////////////////////////////////////////////////////////

static int get_texture_unit (size_t index) {
  switch (index) {
  case 0: return GL_TEXTURE0;
  case 1: return GL_TEXTURE1;
  case 2: return GL_TEXTURE2;
  case 3: return GL_TEXTURE3;
  case 4: return GL_TEXTURE4;
  case 5: return GL_TEXTURE5;
  case 6: return GL_TEXTURE6;
  case 7: return GL_TEXTURE7;
  default: return -1;
  }
}

static int get_num_attrib_components (GLenum type) {
  switch ( type ) {
  case GL_FLOAT:
    return 1;
      
  case GL_FLOAT_VEC2:
    return 2;

  case GL_FLOAT_VEC3:
    return 3;

  case GL_FLOAT_VEC4:
    return 4;

  case GL_FLOAT_MAT2:
    return 4;

  case GL_FLOAT_MAT3:
    return -1;

  case GL_FLOAT_MAT4:
    return -1;
  }
  return -1;
}

static SceneData* alloc_scene_data (Scene* scene) {
  char key[KEYSIZE];
  snprintf (key, KEYSIZE, "%p", scene);
  SceneData* sd = malloc (sizeof (SceneData));

  sd->mapShaderShader2Handle = hashmap_alloc();
  sd->mapVShader2Handle = hashmap_alloc();
  sd->mapFShader2Handle = hashmap_alloc();
  sd->mapBuffer2Handle = hashmap_alloc();
  sd->mapTexture2Handle = hashmap_alloc();

  
  sd->listVShaders = list_alloc();
  sd->listFShaders = list_alloc();
  sd->listPrograms = list_alloc();
  sd->listBuffers = list_alloc();
  sd->listTextures = list_alloc();

  sd->countProgramUsers = hashmap_alloc();
  sd->countVShaderUsers = hashmap_alloc();
  sd->countFShaderUsers = hashmap_alloc();
  sd->countBufferUsers = hashmap_alloc();
  sd->countTextureUsers = hashmap_alloc();

  sd->mapSo2SoData = hashmap_alloc();

  sd->stageGbuffer = list_alloc();
  sd->stageLight = list_alloc();
  sd->stageGeometry = list_alloc();
  sd->stageParticle = list_alloc();
  sd->stageOverlay = list_alloc();

  hashmap_insert (key, sd, mapSceneData);
  
  return sd;
}

static void free_scene_data (Scene* scene) {

  char sdkey[KEYSIZE];
  snprintf (sdkey, KEYSIZE, "%p", scene);
  
  SceneData* sceneData = hashmap_find (sdkey, mapSceneData);

  if (sceneData) {    
      
    GLuint handle;
    
    int k;
    for (k = 0; k < list_size(sceneData->listBuffers); k++) {

      handle = (GLuint)list_get(k, sceneData->listBuffers);
      glDeleteBuffers (1, &handle);
    }

    for ( k = 0; k < list_size(sceneData->listPrograms); k++) {
      
      handle = (GLuint)list_get(k, sceneData->listPrograms);
      glDeleteProgram ( handle );
    }

    for ( k = 0; k < list_size(sceneData->listVShaders); k++) {
      
      handle = (GLuint)list_get(k, sceneData->listVShaders);
      glDeleteShader ( handle );
    }

    for ( k = 0; k < list_size(sceneData->listFShaders); k++) {
      
      handle = (GLuint)list_get(k, sceneData->listFShaders);
      glDeleteShader ( handle );
    }

    for ( k = 0; k < list_size(sceneData->listTextures); k++) {
      
      handle = (GLuint)list_get(k, sceneData->listTextures);
      glDeleteTextures ( 1, &handle );
    }

    list_free (sceneData->listPrograms);
    list_free (sceneData->listVShaders);
    list_free (sceneData->listFShaders);
    list_free (sceneData->listBuffers);
    list_free (sceneData->listTextures);

    hashmap_free (sceneData->mapVShader2Handle);
    hashmap_free (sceneData->mapFShader2Handle);
    hashmap_free (sceneData->mapShaderShader2Handle);
    hashmap_free (sceneData->mapBuffer2Handle);
    hashmap_free (sceneData->mapTexture2Handle);

    hashmap_free (sceneData->countBufferUsers);
    hashmap_free (sceneData->countVShaderUsers);
    hashmap_free (sceneData->countFShaderUsers);
    hashmap_free (sceneData->countProgramUsers);
    hashmap_free (sceneData->countTextureUsers);

    hashmap_free (sceneData->mapSo2SoData);

    list_free (sceneData->stageGbuffer);
    list_free (sceneData->stageLight);
    list_free (sceneData->stageGeometry);
    list_free (sceneData->stageParticle);
    list_free (sceneData->stageOverlay);
    
    hashmap_delete (sdkey, mapSceneData);
  }
}

static SoData* find_so_data (SceneData* sceneData, SceneObject* so) {
  char key[KEYSIZE];
  snprintf (key, KEYSIZE, "%p", so);

  return hashmap_find (key, sceneData->mapSo2SoData);
}

static SceneData* find_scene_data (Scene* scene) {
  char key[KEYSIZE];
  snprintf (key, KEYSIZE, "%p", scene);

  return hashmap_find (key, mapSceneData);
}

static int link_program (GLuint programObject, const char* vShader, const char* fShader) {

  GLint linked;
  glLinkProgram ( programObject );

  // Check the link status
  glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

  if ( !linked ) {
    GLint infoLen = 0;

    glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );

    if ( infoLen > 1 ) {
      char* infoLog = malloc (sizeof(char) * infoLen );

      glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
      log_message ( "Error linking program: %s\n\nvShader:\n%s\n\nfShader:\n%s\n", infoLog, vShader, fShader);

      free ( infoLog );
    }    
    return -1;
  }
  return 0;
}


static GLuint create_program (const char* vShader, const char* fShader, GLuint vShaderHandle, GLuint fShaderHandle) {
  // Create the program object
  GLuint programObject = glCreateProgram ( );

  if ( programObject == 0 ) return -1;

  glAttachShader ( programObject, vShaderHandle );
  glAttachShader ( programObject, fShaderHandle );

  if ( -1 == link_program ( programObject, vShader, fShader ) ) return -1;

  return programObject;
}

static int compile_shader ( GLenum type, const char* shader, GLuint* handle ) {

  GLint compiled;

  // Create the shader object
  *handle = glCreateShader ( type );

  if ( *handle == 0 ) return -1;

  // Load the shader source
  glShaderSource ( *handle, 1, &shader, NULL );

  // Compile the shader
  glCompileShader ( *handle );

  // Check the compile status
  glGetShaderiv ( *handle, GL_COMPILE_STATUS, &compiled );

  if ( !compiled )  {
    GLint infoLen = 0;

    glGetShaderiv ( *handle, GL_INFO_LOG_LENGTH, &infoLen );

    if ( infoLen > 1 ) {
      char* infoLog = malloc (sizeof(char) * infoLen );

      glGetShaderInfoLog ( *handle, infoLen, NULL, infoLog );
      log_message ( "Error compiling shader:\n%s", infoLog );

      free ( infoLog );
    }

    glDeleteShader ( *handle );
    return -1;
  }

  return 0;
}

static int setup_shaders(SceneData* sceneData, SceneObject* so, SoData* soData, GLuint* vShaderHandle, GLuint* fShaderHandle) {

  // lookup vshader
  char shaderKey[KEYSIZE];
  snprintf (shaderKey, KEYSIZE, "%p", &so->vShader);

  size_t shaderUsers;

  GLuint vHandle = (GLuint)hashmap_find (shaderKey, sceneData->mapVShader2Handle);
  if ( !vHandle ) {
    if ( -1 == compile_shader ( GL_VERTEX_SHADER, so->vShader, &vHandle ) ) return -1;

    hashmap_insert(shaderKey, (void*)vHandle, sceneData->mapVShader2Handle);
    list_add((void*)vHandle, sceneData->listVShaders);
    hashmap_insert (shaderKey, (void*)1, sceneData->countVShaderUsers);
    
  } else {
    shaderUsers = (size_t)hashmap_find(shaderKey, sceneData->countVShaderUsers);
    hashmap_insert (shaderKey, (void*)(shaderUsers+1), sceneData->countVShaderUsers);    
  }
  *vShaderHandle = vHandle;

  // lookup fshader
  snprintf (shaderKey, KEYSIZE, "%p", &so->fShader);
  GLuint fHandle = (GLuint)hashmap_find (shaderKey, sceneData->mapFShader2Handle);
  if ( !fHandle ) {
    if ( -1 == compile_shader ( GL_FRAGMENT_SHADER, so->fShader, &fHandle ) ) return -1;
    
    hashmap_insert(shaderKey, (void*)fHandle, sceneData->mapFShader2Handle);
    list_add((void*)fHandle, sceneData->listFShaders);
    hashmap_insert (shaderKey, (void*)1, sceneData->countFShaderUsers);

  } else {
    shaderUsers = (size_t)hashmap_find(shaderKey, sceneData->countFShaderUsers);
    hashmap_insert (shaderKey, (void*)(shaderUsers+1), sceneData->countFShaderUsers);
  }
  *fShaderHandle = fHandle;

  return 0;
}

static int tear_down_shaders (SceneData* sceneData, SceneObject* so, SoData* soData) {

  char shaderKey[KEYSIZE];
  snprintf ( shaderKey, KEYSIZE, "%p", &so->vShader );

  GLuint handle = (GLuint)hashmap_find ( shaderKey, sceneData->mapVShader2Handle );
  size_t shaderUsers = (size_t)hashmap_find ( shaderKey, sceneData->countVShaderUsers );
  shaderUsers--;

  if ( 0 == shaderUsers ) {

    glDeleteShader ( handle );

    list_remove_elem ( (void*)handle, sceneData->listVShaders );
    hashmap_delete ( shaderKey, sceneData->mapVShader2Handle );
    hashmap_delete ( shaderKey, sceneData->countVShaderUsers );

  } else {

    hashmap_insert ( shaderKey, (void*)shaderUsers, sceneData->countVShaderUsers );
  }


  snprintf ( shaderKey, KEYSIZE, "%p", &so->fShader );
  handle = (GLuint)hashmap_find ( shaderKey, sceneData->mapFShader2Handle );
  shaderUsers = (GLuint)hashmap_find ( shaderKey, sceneData->countFShaderUsers );
  shaderUsers--;

  if ( 0 == shaderUsers ) {

    glDeleteShader ( handle );

    list_remove_elem ( (void*)handle, sceneData->listFShaders );
    hashmap_delete ( shaderKey, sceneData->mapFShader2Handle );
    hashmap_delete ( shaderKey, sceneData->countFShaderUsers );

  } else {

    hashmap_insert ( shaderKey, (void*)shaderUsers, sceneData->countFShaderUsers );
  }

  return 0;
}

static int setup_program(SceneData* sceneData, SceneObject* so, SoData* soData, GLuint* programHandle) {

  GLuint vShaderHandle;
  GLuint fShaderHandle;
  if ( -1 == setup_shaders( sceneData, so, soData, &vShaderHandle, &fShaderHandle ) ) return -1;

  char progKey[KEYSIZE];
  snprintf (progKey, KEYSIZE, "%p,%p", &so->vShader, &so->fShader);

  GLuint program = (GLuint)hashmap_find(progKey, sceneData->mapShaderShader2Handle);  
  size_t programUsers;
  if (program) {

    *programHandle = program;
    programUsers = (size_t)hashmap_find ( progKey, sceneData->countProgramUsers );
    hashmap_insert ( progKey, (void*)(programUsers+1), sceneData->countProgramUsers );   
  } else {

    program = create_program( so->vShader, so->fShader, vShaderHandle, fShaderHandle );
    if ( -1 == program ) return -1;
    
    list_add ( (void*)program, sceneData->listPrograms );
    hashmap_insert ( progKey, (void*)program, sceneData->mapShaderShader2Handle );
    hashmap_insert ( progKey, (void*)1, sceneData->countProgramUsers );
    
    *programHandle = program;
  }
  return 0;
}

static int tear_down_program(SceneData* sceneData, SceneObject* so, SoData* soData) {

  tear_down_shaders ( sceneData, so, soData );

  char progKey[KEYSIZE];
  snprintf (progKey, KEYSIZE, "%p,%p", &so->vShader, &so->fShader);

  GLuint program = (GLuint)hashmap_find(progKey, sceneData->mapShaderShader2Handle);  
  size_t programUsers = (size_t)hashmap_find ( progKey, sceneData->countProgramUsers );
  programUsers--;

  if ( 0 == programUsers ) {

    glDeleteProgram ( program );

    hashmap_delete ( progKey, sceneData->mapShaderShader2Handle );
    hashmap_delete ( progKey, sceneData->countProgramUsers );
    list_remove_elem ( (void*)program, sceneData->listPrograms );

  } else {
    hashmap_insert ( progKey, (void*)programUsers, sceneData->countProgramUsers );
  }

  return 0;
}

static int setup_attributes (SceneData* sceneData, SceneObject* so, SoData* soData, GLuint program) {

  GLint num;
  char name_i[MAX_NAME_LENGTH];
  GLenum type_i;
  GLint size_i;

  Attribute* attribute;
  char bufferKey[KEYSIZE]; 
  size_t bufferUsers;
  GLuint bufferHandle;
 
  size_t numAttrComponents;


  glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &num);
      
  int i;
  for (i = 0; i < num; i++) {
    glGetActiveAttrib(program, i, MAX_NAME_LENGTH, NULL, &size_i, &type_i, name_i);

    numAttrComponents = get_num_attrib_components(type_i);

    if ( -1 == numAttrComponents ) {
      log_message ("error: matrix 3x3 and matrix 4x4 attributes are not supported");
      return -1;
    }

    attribute = hashmap_find(name_i, so->attributes);

    if ( !attribute ) {
      log_message ("error: shader attribute [%s] not found in so attribute map.", name_i);
      return -1;
    }

    list_add( attribute, soData->attributes );

    if ( 1 == attribute->useBuffer ) { // BUFFERED ATTRIBUTES    
            
      snprintf (bufferKey, KEYSIZE, "%p", attribute);
      bufferHandle = (GLuint)hashmap_find ( bufferKey, sceneData->mapBuffer2Handle );      

      if ( !bufferHandle ) { // create new vbo
	glGenBuffers(1, &bufferHandle);	

	glBindBuffer (GL_ARRAY_BUFFER, bufferHandle);

	glBufferData (GL_ARRAY_BUFFER, attribute->size, attribute->data, GL_STATIC_DRAW);

	glBindBuffer (GL_ARRAY_BUFFER, 0);

	// update sceneData
	list_add ( (void*)bufferHandle, sceneData->listBuffers );	
	hashmap_insert ( bufferKey, (void*)bufferHandle, sceneData->mapBuffer2Handle );
	hashmap_insert ( bufferKey, (void*)1, sceneData->countBufferUsers );

      } else { 

	// update sceneData	
	bufferUsers = (size_t)hashmap_find (bufferKey, sceneData->countBufferUsers);
	hashmap_insert (bufferKey, (void*)(bufferUsers+1), sceneData->countBufferUsers);	
      }


    } else { // UNBUFFERD ATTRIBUTES

      // do nothing

    }

  }

  //if ( -1 == link_program ( program, so->vShader, so->fShader ) ) return -1; // binding requires relinking

  return 0;
}

static int tear_down_attributes (SceneData* sceneData, SceneObject* so, SoData* soData) {
  Attribute* attribute;
  char bufferKey[KEYSIZE];
  size_t bufferUsers;
  GLuint bufferHandle;

  int i;
  for (i = 0; i < list_size(soData->attributes); i++) {

    attribute = list_get ( i, soData->attributes );

    if ( 1 != attribute->useBuffer ) continue;

    snprintf (bufferKey, KEYSIZE, "%p", attribute);
    bufferHandle = (GLuint)hashmap_find ( bufferKey, sceneData->mapBuffer2Handle );
    
    bufferUsers = (size_t)hashmap_find ( bufferKey, sceneData->countBufferUsers );
    bufferUsers--;
    if ( 0 == bufferUsers) { // free resources

      glDeleteBuffers (1, &bufferHandle);

      list_remove_elem ( (void*)bufferHandle, sceneData->listBuffers );

      hashmap_delete ( bufferKey, sceneData->mapBuffer2Handle );

      hashmap_delete ( bufferKey, sceneData->countBufferUsers );

    } else {

      hashmap_insert ( bufferKey, (void*)bufferUsers, sceneData->countBufferUsers );
    }
  }

  return 0;
}

static int setup_textures (SceneData* sceneData, SceneObject* so, SoData* soData, GLuint program) {
  
  GLint num;

  GLint size_i;
  GLenum type_i;
  char name_i[MAX_NAME_LENGTH];
  GLint location_i;
  Uniform* uniform;
  GLuint textureHandle;
  size_t textureUsers;
  char uniKey[KEYSIZE];

  uint i;
  glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &num);

  for (i = 0; i < num; i++) {
    glGetActiveUniform(program, i, MAX_NAME_LENGTH, NULL, &size_i, &type_i, name_i);
    location_i = glGetUniformLocation(program, name_i);

    if ( -1 == location_i) {
      log_message ("error: could not find name: [%s] in shader program.");
      return -1;
    }
    
    uniform = hashmap_find(name_i, so->uniforms);
    if ( !uniform ) {
      log_message ("error: could not find [%s] in scene object's uniform map", name_i);
      return -1;
    }

    if ( GL_SAMPLER_CUBE == type_i ) {
      log_message ( "error: cube maps are not supported" );
      return -1;
    }

    if ( GL_SAMPLER_2D != type_i) {
      continue;
    }

    // type_i == GL_SAMPLER_2D

    snprintf ( uniKey, KEYSIZE, "%p", uniform );
    textureHandle = (GLuint)hashmap_find ( uniKey, sceneData->mapTexture2Handle );


    if ( textureHandle ) { // texture already exists

      textureUsers = (size_t)hashmap_find ( uniKey, sceneData->countTextureUsers );
      hashmap_insert ( uniKey, (void*)(textureUsers+1), sceneData->countTextureUsers );

    } else { // texture does not exist

      if ( 0 != uniform->textureHandle ) { // just map to handle

	if ( TEXTURE_HANDLE_GBUFFER == uniform->textureHandle ) {
	  textureHandle = sceneData->tbos[0];
	}
	else if ( TEXTURE_HANDLE_LIGHTBUFFER == uniform->textureHandle ) {
	  textureHandle = sceneData->tbos[1];
	}
	else {
	  textureHandle = uniform->textureHandle;	
	}

      } else { // load texture and map
      
	glGenTextures ( 1, &textureHandle );

	glBindTexture ( GL_TEXTURE_2D, textureHandle );

	glTexImage2D( GL_TEXTURE_2D, 0, uniform->textureFormat, uniform->width, uniform->height, 0, uniform->textureFormat, uniform->textureType, uniform->data);       

	glGenerateMipmap ( GL_TEXTURE_2D );

	glBindTexture ( GL_TEXTURE_2D, 0 );

	hashmap_insert ( uniKey, (void*)textureHandle, sceneData->mapTexture2Handle );
      }

      hashmap_insert ( uniKey, (void*)textureHandle, sceneData->mapTexture2Handle );
      hashmap_insert ( uniKey, (void*)1, sceneData->countTextureUsers );
      list_add ( (void*)textureHandle, sceneData->listTextures );
    }
    
    if ( TEXTURE_HANDLE_GBUFFER != uniform->textureHandle && TEXTURE_HANDLE_LIGHTBUFFER != uniform->textureHandle ) {
      list_add ( uniform, soData->textures );
    }
  }


  return 0;
}

static int tear_down_textures (SceneData* sceneData, SceneObject* so, SoData* soData) {
  
  Uniform* uniform;
  char uniKey[KEYSIZE];
  GLuint textureHandle;
  size_t textureUsers;

  int i;
  for ( i = 0; i < list_size ( soData->textures ); i++) {
    
    uniform = list_get( i, soData->textures);
    snprintf ( uniKey, KEYSIZE, "%p", uniform );    
    textureHandle = (GLuint)hashmap_find ( uniKey, sceneData->mapTexture2Handle );
    textureUsers = (size_t)hashmap_find ( uniKey, sceneData->countTextureUsers );
    
    textureUsers--;

    if ( 0 == textureUsers ) {
      glDeleteTextures( 1, &textureHandle );
      
      hashmap_delete ( uniKey, sceneData->mapTexture2Handle );
      hashmap_delete ( uniKey, sceneData->countTextureUsers );
      list_remove_elem ( (void*)textureHandle, sceneData->listTextures );

    } else {
      hashmap_insert ( uniKey, (void*)textureUsers, sceneData->countTextureUsers );
    }
  }

  return 0;
}

static int setup_index_buffer (SceneData* sceneData, SceneObject* so, SoData* soData) {
  Draw* method = so->draw;

  char bufferKey[KEYSIZE];
  Attribute* indices = method->indices;
  GLuint bufferHandle;
  size_t bufferUsers;

  if ( indices && 1 == indices->useBuffer ) {

    snprintf (bufferKey, KEYSIZE, "%p", indices);
    bufferHandle = (GLuint)hashmap_find ( bufferKey, sceneData->mapBuffer2Handle );

    if ( !bufferHandle ) {
      glGenBuffers(1, &bufferHandle);

      glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, bufferHandle);

      glBufferData (GL_ELEMENT_ARRAY_BUFFER, indices->size, indices->data, GL_STATIC_DRAW);

      glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);

      // update sceneData
      list_add ( (void*)bufferHandle, sceneData->listBuffers );	
      hashmap_insert ( bufferKey, (void*)bufferHandle, sceneData->mapBuffer2Handle );
      hashmap_insert ( bufferKey, (void*)1, sceneData->countBufferUsers );

    } else { 

      // update sceneData	
      bufferUsers = (size_t)hashmap_find (bufferKey, sceneData->countBufferUsers);
      hashmap_insert (bufferKey, (void*)(bufferUsers+1), sceneData->countBufferUsers);	
    }
  }

  return 0;
}

static int tear_down_index_buffer (SceneData* sceneData, SceneObject* so, SoData* soData) {
  Draw* method = so->draw;

  char bufferKey[KEYSIZE];

  Attribute* indices = method->indices;
  GLuint bufferHandle;
  size_t bufferUsers;

  if ( indices && 1 == indices->useBuffer ) {
    snprintf (bufferKey, KEYSIZE, "%p", indices);
    bufferUsers = (size_t)hashmap_find ( bufferKey, sceneData->countBufferUsers );
    bufferUsers--;

    if ( 0 == bufferUsers ) {

      bufferHandle = (GLuint)hashmap_find ( bufferKey, sceneData->mapBuffer2Handle );
      glDeleteBuffers ( 1, &bufferHandle );
      hashmap_delete ( bufferKey, sceneData->mapBuffer2Handle );
      hashmap_delete ( bufferKey, sceneData->countBufferUsers );
      list_remove_elem ( (void*)bufferHandle, sceneData->listBuffers );

    } else {
      
      hashmap_insert ( bufferKey, (void*)bufferUsers, sceneData->countBufferUsers );
    }

  }

  return 0;
}

static int setup_so (SceneData* sceneData, SceneObject* so) {

  SoData* soData = malloc(sizeof(SoData));

  soData->attributes = list_alloc();
  soData->textures = list_alloc();

  GLuint programHandle;

  if ( -1 == setup_program( sceneData, so, soData, &programHandle ) ) return -1;

  if ( -1 == setup_attributes( sceneData, so, soData, programHandle ) ) return -1;  

  if ( -1 == setup_textures ( sceneData, so, soData, programHandle ) ) return -1;

  if ( -1 == setup_index_buffer ( sceneData, so, soData ) ) return -1;

  char soKey[KEYSIZE];
  snprintf ( soKey, KEYSIZE, "%p", so );
  hashmap_insert ( soKey, soData, sceneData->mapSo2SoData );

  return 0;  
}

static int tear_down_so (SceneData* sceneData, SceneObject* so) {

  SoData* soData = find_so_data ( sceneData, so );

  if ( !soData ) return -1;

  tear_down_index_buffer ( sceneData, so, soData );
  tear_down_textures ( sceneData, so, soData );
  tear_down_attributes ( sceneData, so, soData );
  tear_down_program ( sceneData, so, soData );

  list_free ( soData->attributes );
  list_free ( soData->textures );

  char soKey[KEYSIZE];
  snprintf ( soKey, KEYSIZE, "%p", so );
  hashmap_delete ( soKey, sceneData->mapSo2SoData ) ;

  free ( soData );

  return 0;
}

static int find_and_pass_uniforms(SceneData* sceneData, SceneObject* so, GLuint program) {

  GLint num;

  GLint size_i;
  GLenum type_i;
  char name_i[MAX_NAME_LENGTH];
  GLint location_i;
  Uniform* uniform = NULL;
  void* data = NULL;
  char texKey[KEYSIZE];
  GLuint textureHandle;

  size_t textureIndex = 0;

  uint i;

  glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &num);

  for (i = 0; i < num; i++) {
    glGetActiveUniform(program, i, MAX_NAME_LENGTH, NULL, &size_i, &type_i, name_i);
    location_i = glGetUniformLocation(program, name_i);

    if ( -1 == location_i) {
      log_message ("error: could not find name: [%s] in shader program.", name_i);
      return -1;
    }

    uniform = hashmap_find(name_i, so->uniforms);
    if ( !uniform ) {
      log_message ("error: could not find [%s] in scene object's uniform map", name_i);
      return -1;
    }

    data = uniform->data;


    switch (type_i) {
    case GL_BOOL:
      if (  1 == size_i ) glUniform1i(location_i, ((GLint*)data)[0]);
      else glUniform1iv(location_i, size_i, ((const GLint*)data));
      break;

    case GL_BOOL_VEC2:
      if ( 1 == size_i ) glUniform2i(location_i, ((GLint*)data)[0], ((GLint*)data)[1]);
      else glUniform2iv(location_i, size_i, ((const GLint*)data));
      break;

    case GL_BOOL_VEC3:
      if ( 1 == size_i ) glUniform3i(location_i, ((GLint*)data)[0], ((GLint*)data)[1], ((GLint*)data)[2]);
      else glUniform3iv(location_i, size_i, ((const GLint*)data));
      break;

    case GL_BOOL_VEC4:
      if ( 1 == size_i ) glUniform4i(location_i, ((GLint*)data)[0], ((GLint*)data)[1], ((GLint*)data)[2], ((GLint*)data)[3]);
      else glUniform4iv(location_i, size_i, ((const GLint*)data));
      break;

    case GL_INT:
      if ( 1 == size_i ) glUniform1i(location_i, (GLint)data);
      else glUniform1iv(location_i, size_i, ((const GLint*)data));
      break;

    case GL_INT_VEC2:
      if ( 1 == size_i ) glUniform2i(location_i, ((GLint*)data)[0], ((GLint*)data)[1]);
      else glUniform2iv(location_i, size_i, ((const GLint*)data));
      break;

    case GL_INT_VEC3:
      if ( 1 == size_i ) glUniform3i(location_i, ((GLint*)data)[0], ((GLint*)data)[1], ((GLint*)data)[2]);
      else glUniform3iv(location_i, size_i, ((const GLint*)data));
      break;

    case GL_INT_VEC4:
      if ( 1 == size_i ) glUniform4i(location_i, ((GLint*)data)[0], ((GLint*)data)[1], ((GLint*)data)[2], ((GLint*)data)[3]);
      else glUniform4iv(location_i, size_i, ((const GLint*)data));
      break;

    case GL_FLOAT:
      if ( 1 == size_i ) glUniform1f(location_i, ((GLfloat*)data)[0]);
      else glUniform1fv(location_i, size_i, ((const GLfloat*)data));
      break;

    case GL_FLOAT_VEC2:
      if ( 1 == size_i ) glUniform2f(location_i, ((GLfloat*)data)[0], ((GLfloat*)data)[1]);
      else glUniform2fv(location_i, size_i, ((const GLfloat*)data));
      break;

    case GL_FLOAT_VEC3:
      if ( 1 == size_i ) glUniform3f(location_i, ((GLfloat*)data)[0], ((GLfloat*)data)[1], ((GLfloat*)data)[2]);
      else glUniform3fv(location_i, size_i, ((const GLfloat*)data));
      break;

    case GL_FLOAT_VEC4:
      if ( 1 == size_i ) glUniform4f(location_i, ((GLfloat*)data)[0], ((GLfloat*)data)[1], ((GLfloat*)data)[2], ((GLfloat*)data)[3]);      
      else glUniform4fv(location_i, size_i, ((const GLfloat*)data));
      break;

    case GL_FLOAT_MAT2:
      glUniformMatrix2fv(location_i, size_i, GL_FALSE, (const GLfloat*)data);
      break;

    case GL_FLOAT_MAT3:
      glUniformMatrix3fv(location_i, size_i, GL_FALSE, (const GLfloat*)data);
      break;
      
    case GL_FLOAT_MAT4:
      glUniformMatrix4fv(location_i, size_i, GL_FALSE, (const GLfloat*)data);
      break;

    case GL_SAMPLER_2D:

      snprintf( texKey, KEYSIZE, "%p", uniform );      
      textureHandle = (GLuint)hashmap_find ( texKey, sceneData->mapTexture2Handle );

      if ( !textureHandle ) {
	log_message ( "error: could not bind texture. handle not found");
	return -1;
      }
      
      if (textureIndex > 7) {
	log_message ( "error: cannot use more than 8 texture units per scene object" );
	return -1;
      }

      
      glActiveTexture(get_texture_unit(textureIndex));

      glBindTexture (GL_TEXTURE_2D, textureHandle);

      glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, uniform->sampleMode );
      glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, uniform->sampleMode );

      
      glUniform1i ( location_i, textureIndex );
      
      textureIndex++;

      break;

    case GL_SAMPLER_CUBE:
      log_message ( "error: cube maps are not supported" );
      break;
    }
  }

  return 0;
}

///////////////////////////////////////////////////////////////
///////////    API
///////////////////////////////////////////////////////////////

int renderman_initialize () {
  mapSceneData = hashmap_alloc();
  staticPrograms = hashmap_alloc();
  staticShaders = hashmap_alloc();

  return 0;
}

int renderman_tear_down () {
  hashmap_free ( mapSceneData );
  hashmap_free ( staticShaders );
  hashmap_free ( staticPrograms );
  return 0;
}

void renderman_snapshot(Scene* scene, size_t x, size_t y, size_t width, size_t height, char* buffer) {

  SceneData* sceneData = find_scene_data ( scene );

  if ( sceneData ) {

    glReadPixels ( x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer );

  }
}

int renderman_add_so(Scene* scene, SceneObject* so) {
  
  SceneData* sceneData = find_scene_data ( scene );

  if ( 1 == scene->running && sceneData ) {
    if ( -1 == setup_so ( sceneData, so ) ) {

      log_message ( "error setting up so. removing." );
      
      tear_down_so ( sceneData, so );

    } else {

      if ( (1 & so->stage) == 1) list_add( so, sceneData->stageGbuffer);
      if ( (2 & so->stage) == 2) list_add( so, sceneData->stageLight);
      if ( (4 & so->stage) == 4) list_add( so, sceneData->stageGeometry);
      if ( (8 & so->stage) == 8) list_add( so, sceneData->stageParticle);
      if ( (16 & so->stage) == 16) list_add( so, sceneData->stageOverlay);

    }
  } else {

    list_add( so, scene->initialObjects );

  }
  
  return 0;
}


int renderman_remove_so(Scene* scene, SceneObject* so) {
  
  SceneData* sceneData = find_scene_data ( scene );

  if ( 1 == scene->running ) {

    if ( !sceneData ) return -1;
    
    SoData* soData = find_so_data ( sceneData, so );

    if ( !soData ) return -1;
    
    list_remove_elem( so, sceneData->stageGbuffer );  
    list_remove_elem( so, sceneData->stageLight );
    list_remove_elem( so, sceneData->stageGeometry );
    list_remove_elem( so, sceneData->stageParticle );
    list_remove_elem( so, sceneData->stageOverlay );

    tear_down_so ( sceneData, so );  

  } else {
    list_remove_elem ( so, scene->initialObjects );
  }

  return 0;
}


size_t renderman_info_num_vshaders(Scene* scene) {
  SceneData* sceneData = find_scene_data ( scene );
  return list_size ( sceneData->listVShaders );
}

size_t renderman_info_num_fshaders(Scene* scene) {
  SceneData* sceneData = find_scene_data ( scene );
  return list_size ( sceneData->listFShaders );
}

size_t renderman_info_num_programs(Scene* scene) {
  SceneData* sceneData = find_scene_data ( scene );
  return list_size ( sceneData->listPrograms );
}

size_t renderman_info_num_buffers(Scene* scene) {
  SceneData* sceneData = find_scene_data ( scene );
  return list_size ( sceneData->listBuffers );
}

size_t renderman_info_num_textures(Scene* scene) {
  SceneData* sceneData = find_scene_data ( scene );
  return list_size ( sceneData->listTextures );
}


int renderman_rendertask_setup(RenderContext* ctx) {

  if (EGL_TRUE != eglMakeCurrent(ctx->esContext->display, ctx->esContext->surface, ctx->esContext->surface, ctx->esContext->context)) return -1;

  SceneData* sceneData = alloc_scene_data ( ctx->scene );

  glViewport ( 0, 0, ctx->esContext->screenWidth, ctx->esContext->screenHeight );

  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei (GL_PACK_ALIGNMENT, 1);

  glFrontFace(GL_CW);
  glCullFace(GL_BACK);

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  glGenFramebuffers(2, sceneData->fbos);
  glGenRenderbuffers(1, sceneData->rbos);
  glGenTextures(2, sceneData->tbos);
  GLenum status;

  // FBO 0  
  glBindTexture(GL_TEXTURE_2D, sceneData->tbos[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ctx->esContext->screenWidth, ctx->esContext->screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);

  // RBO 0
  glBindRenderbuffer(GL_RENDERBUFFER, sceneData->rbos[0]);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, ctx->esContext->screenWidth, ctx->esContext->screenHeight);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);  

  glBindFramebuffer(GL_FRAMEBUFFER, sceneData->fbos[0]);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneData->tbos[0], 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, sceneData->rbos[0]);

  status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if ( GL_FRAMEBUFFER_COMPLETE != status ) {
    log_message ("error: could not set up gbuffer");
    return -1;
  }  

  // FBO 1 
  glBindTexture(GL_TEXTURE_2D, sceneData->tbos[1]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ctx->esContext->screenWidth, ctx->esContext->screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);

  glBindFramebuffer(GL_FRAMEBUFFER, sceneData->fbos[1]);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneData->tbos[1], 0);
  
  status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if ( GL_FRAMEBUFFER_COMPLETE != status ) {
    log_message ("error: could not set up light buffer");
    return -1;
  }


  // compile static shaders
  GLuint vHandle;
  GLuint fHandle;
  GLuint pHandle;
  
  if ( -1 == compile_shader ( GL_VERTEX_SHADER, vGShader, &vHandle ) ) return -1;
  hashmap_insert( "vGShader", (void*)vHandle, staticShaders );
  
  if ( -1 == compile_shader ( GL_FRAGMENT_SHADER, fGShader, &fHandle ) ) return -1;
  hashmap_insert( "fGShader", (void*)fHandle, staticShaders );

  pHandle = create_program (vGShader, fGShader, vHandle, fHandle);
  if ( -1 == pHandle ) return -1;
  hashmap_insert( "gbuffer", (void*)pHandle, staticPrograms );

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  int i;
  SceneObject* so;
  for ( i = list_size ( ctx->scene->initialObjects ) - 1; i >= 0; i-- ) {

    so = list_get ( i, ctx->scene->initialObjects );
    if ( -1 == setup_so ( sceneData, so ) ) {

      log_message ( "error setting up so. removing." );
      
      tear_down_so ( sceneData, so );

    } else {

      if ( (1 & so->stage) == 1) list_add( so, sceneData->stageGbuffer);
      if ( (2 & so->stage) == 2) list_add( so, sceneData->stageLight);
      if ( (4 & so->stage) == 4) list_add( so, sceneData->stageGeometry);
      if ( (8 & so->stage) == 8) list_add( so, sceneData->stageParticle);
      if ( (16 & so->stage) == 16) list_add( so, sceneData->stageOverlay);

    }
  }
  list_clear ( ctx->scene->initialObjects );
  
  return 0;
}

static inline void render_stage(List stage, SceneData* sceneData, SceneObject* so, SoData* soData, char* progKey, Attribute* attribute, size_t numAttributes, GLuint program, char* bufferKey, GLuint bufferHandle, char* iboKey, GLuint iboHandle, Draw* method, int i, int k) {
  
  for (i = list_size(stage) - 1; i >= 0; i--) {   

    so = list_get(i, stage);    

    soData = find_so_data ( sceneData, so );

    if ( !soData ) {
      log_message ( "error: scene object has not been setup correctly - removing." ); 
      tear_down_so ( sceneData, so );
      list_remove_elem( so, stage );
      continue;
    }
	
    snprintf(progKey, KEYSIZE, "%p,%p", &so->vShader, &so->fShader);
    program = (GLuint)hashmap_find( progKey, sceneData->mapShaderShader2Handle );

    glUseProgram ( program );

    if ( -1 == find_and_pass_uniforms (sceneData, so, (GLuint)program)) {
      log_message("error: scene object uniforms have not been setup correctly - removing so.");
      tear_down_so ( sceneData, so );
      list_remove_elem( so, stage );
      continue;
    }

    numAttributes = list_size (soData->attributes);

    for (k = 0; k < numAttributes; k++) {

      attribute = list_get ( k, soData->attributes );

      if ( 1 == attribute->useBuffer ) {

	snprintf ( bufferKey, KEYSIZE, "%p", attribute );
	bufferHandle = (GLuint)hashmap_find ( bufferKey, sceneData->mapBuffer2Handle );

	glBindBuffer (GL_ARRAY_BUFFER, bufferHandle);
	  
	glVertexAttribPointer (k, attribute->numComponents, attribute->dataType, GL_FALSE, 0, 0);
	  
	glEnableVertexAttribArray(k);

      } else {	

	glVertexAttribPointer (k, attribute->numComponents, attribute->dataType, GL_FALSE, 0, attribute->data);
	glEnableVertexAttribArray(k);

      }
    }

    method = so->draw;
    if ( NULL == method->indices ) { 	

      glDrawArrays ( method->type, method->start, method->count );

    } else {

      snprintf( iboKey, KEYSIZE, "%p", method->indices );
      iboHandle = (GLuint)hashmap_find ( iboKey, sceneData->mapBuffer2Handle );
	
      if ( 1 == method->indices->useBuffer ) {
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, iboHandle );
      }

      glDrawElements ( method->type, method->count, method->indices->dataType, 1 == method->indices->useBuffer ? 0 : method->indices->data );
    }

    for (k = 0; k < numAttributes; k++) {
      glDisableVertexAttribArray(k);
    }
    
    glBindBuffer (GL_ARRAY_BUFFER, 0);
    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture (GL_TEXTURE_2D, 0);
  }

}

int renderman_rendertask_repeat(RepeatInfo* info, RenderContext* ctx) {

  SceneData* sceneData = find_scene_data (ctx->scene);

  if ( !sceneData ) {
    log_message("error: scene data is missing");
    return -1;
  }

  SceneObject* so;
  SoData* soData;

  char progKey[KEYSIZE];

  Attribute* attribute;
  size_t numAttributes;

  GLuint program;

  char bufferKey[KEYSIZE];
  GLuint bufferHandle;
  char iboKey[KEYSIZE];
  GLuint iboHandle;

  Draw* method;
  
  int i;
  int k;
  
  //////////////////////////////////////////////////
  //////////////////////// STAGE Gbuffer
  //////////////////////////////////////////////////
  glBindFramebuffer(GL_FRAMEBUFFER, sceneData->fbos[0]);  
  glEnable(GL_DEPTH_TEST);  
  glEnable(GL_CULL_FACE);
  glDisable(GL_BLEND);
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  for (i = list_size(sceneData->stageGbuffer) - 1; i >= 0; i--) {   

    so = list_get(i, sceneData->stageGbuffer);    

    soData = find_so_data ( sceneData, so );

    if ( !soData ) {
      log_message ( "error: scene object has not been setup correctly - removing." ); 
      tear_down_so ( sceneData, so );
      list_remove_elem( so, sceneData->stageGbuffer );
      continue;
    }

    program = (GLuint)hashmap_find( "gbuffer", staticPrograms );

    glUseProgram ( program );

    if ( -1 == find_and_pass_uniforms (sceneData, so, (GLuint)program)) {
      tear_down_so ( sceneData, so );
      list_remove_elem( so, sceneData->stageGbuffer );
      continue;
    }

    for (k = 0; k < 2; k++) { // position and normal must be first 2 shader attributes

      attribute = list_get ( k, soData->attributes );

      if ( 1 == attribute->useBuffer ) {
	snprintf ( bufferKey, KEYSIZE, "%p", attribute );
	bufferHandle = (GLuint)hashmap_find ( bufferKey, sceneData->mapBuffer2Handle );

	glBindBuffer (GL_ARRAY_BUFFER, bufferHandle);
	  
	glVertexAttribPointer (k, attribute->numComponents, attribute->dataType, GL_FALSE, 0, 0);	
	glEnableVertexAttribArray(k);

      } else {

	glVertexAttribPointer (k, attribute->numComponents, attribute->dataType, GL_FALSE, 0, attribute->data);
	glEnableVertexAttribArray(k);

      }
    }

    method = so->draw;
    if ( NULL == method->indices ) { 	
      glDrawArrays ( method->type, method->start, method->count );
    } else {

      snprintf( iboKey, KEYSIZE, "%p", method->indices );
      iboHandle = (GLuint)hashmap_find ( iboKey, sceneData->mapBuffer2Handle );
	
      if ( 1 == method->indices->useBuffer ) {
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, iboHandle );
      }

      glDrawElements ( method->type, method->count, method->indices->dataType, 1 == method->indices->useBuffer ? 0 : method->indices->data );
    }

    
    for (k = 0; k < 2; k++) {
      glDisableVertexAttribArray(k);
    }

    glBindBuffer (GL_ARRAY_BUFFER, 0);
    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture (GL_TEXTURE_2D, 0);
  }
  
  //////////////////////////////////////////////////
  //////////////////////// STAGE Light
  //////////////////////////////////////////////////
  glBindFramebuffer(GL_FRAMEBUFFER, sceneData->fbos[1]);  
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);
  glClear(GL_COLOR_BUFFER_BIT);

  render_stage(sceneData->stageLight, sceneData, so, soData, progKey, attribute, numAttributes, program, bufferKey, bufferHandle, iboKey, iboHandle, method, i, k);  

  //////////////////////////////////////////////////
  //////////////////////// STAGE Geometry
  //////////////////////////////////////////////////  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);  
  glEnable(GL_DEPTH_TEST);  
  glEnable(GL_CULL_FACE);
  glDisable(GL_BLEND);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  render_stage(sceneData->stageGeometry, sceneData, so, soData, progKey, attribute, numAttributes, program, bufferKey, bufferHandle, iboKey, iboHandle, method, i, k);

  //////////////////////////////////////////////////
  //////////////////////// STAGE Particle
  //////////////////////////////////////////////////

  //////////////////////////////////////////////////
  //////////////////////// STAGE Overlay
  //////////////////////////////////////////////////
  glDisable(GL_DEPTH_TEST);  
  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  render_stage(sceneData->stageOverlay, sceneData, so, soData, progKey, attribute, numAttributes, program, bufferKey, bufferHandle, iboKey, iboHandle, method, i, k);  

  //////////////////////////////////////////////////
  //////////////////////// STAGE Postprocessing
  //////////////////////////////////////////////////


  //////////////////////////////////////////////////
  //////////////////////// STAGE Update
  //////////////////////////////////////////////////

  eglSwapBuffers(ctx->esContext->display, ctx->esContext->surface);

  if ( ctx->updateFunc ) {
    ctx->updateFunc(ctx->userData, info->overruns);
  }
 
  return 0;
}

int renderman_rendertask_tear_down(RenderContext* ctx) {

  SceneData* sceneData = find_scene_data( ctx->scene );

  glDeleteFramebuffers(2, sceneData->fbos);
  glDeleteRenderbuffers(1, sceneData->rbos);
  glDeleteTextures(2, sceneData->tbos);

  if (EGL_TRUE != eglMakeCurrent(ctx->esContext->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) return -1;

  free_scene_data ( ctx->scene );
  
  return 0;
}
