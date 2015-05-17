
/* This is auto-generated code. Edit at your own peril. */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "CuTest.h"

#include "gre.h"
#include "types.h"

static const char* vShaderPosition =
    "attribute vec4 position;    \n"
    "void main()                  \n"
    "{                            \n"
    "   gl_Position = position;  \n"
    "}                            \n";

static const char* fShaderColor =
    "precision mediump float;\n"
    "uniform vec4 color\n;"
    "void main()                                  \n"
    "{                                            \n"
    "  gl_FragColor = color;\n"
    "}                                            \n";  


void testDrawArrays(CuTest* tc) {

  gre_initialize();

  GREHandle drawRed = gre_create_draw_arrays(DRAW_TRIANGLES, 0, 6);
  GREHandle quadRed = gre_create_so(16, vShaderPosition, fShaderColor, drawRed);
  GLfloat vVerticesRed[] = {
    -1.0f,  -1.0f, 
    0.0f, -1.0f, 
    -1.0f, 1.0f, 

    -1.0f, 1.0f, 
    0.0f, -1.0f,
    0.0f, 1.0f,
  };
  GREHandle attrRedPos = gre_create_attribute(vVerticesRed, TYPE_FLOAT, sizeof(float)*12, 2, 1);
  gre_so_attribute_set( quadRed, "position", attrRedPos);
  float colorRed[] = {
    1.0f, 0.0f, 0.0f, 1.0f
  };
  GREHandle uniRedColor = gre_create_uniform(colorRed);  
  gre_so_uniform_set ( quadRed, "color", uniRedColor );


  GREHandle drawBlue = gre_create_draw_arrays(DRAW_TRIANGLES, 0, 6);
  GREHandle quadBlue = gre_create_so(16, vShaderPosition, fShaderColor, drawBlue);
  GLfloat vVerticesBlue[] = {
    0.0f, -1.0f,
    1.0f, -1.0f,
    0.0f, 1.0f,
    0.0f, 1.0f, 
    1.0f, -1.0f,
    1.0f, 1.0f,
  };
  GREHandle attrBluePos = gre_create_attribute(vVerticesBlue, TYPE_FLOAT, sizeof(float)*12, 2, 0);
  gre_so_attribute_set ( quadBlue, "position", attrBluePos );

  float colorBlue[] = {
    0.0f, 0.0f, 1.0f, 1.0f
  };
  GREHandle uniBlueColor = gre_create_uniform(colorBlue);
  gre_so_uniform_set ( quadBlue, "color", uniBlueColor );


  GREHandle sceneHandle = gre_scene_alloc();

  gre_scene_add_so (sceneHandle, quadRed );  
  gre_scene_add_so ( sceneHandle, quadBlue );
  

  ESContext esContext;
  escontext_init (&esContext, NULL);

  RenderConfig renderConfig;
  renderConfig.framerate = FRAMERATE_30;
  renderConfig.esContext = &esContext;

  gre_scene_start ( sceneHandle, &renderConfig );

  CuAssertIntEquals ( tc, 1, gre_info_num_vshaders ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 1, gre_info_num_fshaders ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 1, gre_info_num_programs ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 1, gre_info_num_buffers ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 0, gre_info_num_textures ( sceneHandle ) );

  char screenPixels[ 3 * esContext.screenWidth * esContext.screenHeight / 2 ];
   
  int i;  
  gre_scene_snapshot ( sceneHandle, 0, 0, esContext.screenWidth / 2, esContext.screenHeight, screenPixels );
  for (i = 0; i < (3 * esContext.screenWidth * esContext.screenHeight/2) - 2; i+=3) {
    CuAssertIntEquals ( tc, 0xff, screenPixels[i]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+1]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+2]);
  }
  gre_scene_snapshot ( sceneHandle, esContext.screenWidth / 2, 0, esContext.screenWidth / 2, esContext.screenHeight, screenPixels );

  for (i = 0; i < (3 * esContext.screenWidth * esContext.screenHeight/2) - 2; i+=3) {
    CuAssertIntEquals ( tc, 0x00, screenPixels[i]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+1]);
    CuAssertIntEquals ( tc, 0xff, screenPixels[i+2]);
  }
  gre_scene_remove_so ( sceneHandle, quadRed );

  CuAssertIntEquals ( tc, 1, gre_info_num_vshaders ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 1, gre_info_num_fshaders ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 1, gre_info_num_programs ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 0, gre_info_num_buffers ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 0, gre_info_num_textures ( sceneHandle ) );

  gre_scene_snapshot ( sceneHandle, 0, 0, esContext.screenWidth / 2, esContext.screenHeight, screenPixels );
  for (i = 0; i < (3 * esContext.screenWidth * esContext.screenHeight/2) - 2; i+=3) {
    CuAssertIntEquals ( tc, 0x00, screenPixels[i]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+1]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+2]);
  }

  gre_scene_snapshot ( sceneHandle, esContext.screenWidth / 2, 0, esContext.screenWidth / 2, esContext.screenHeight, screenPixels );
  for (i = 0; i < (3 * esContext.screenWidth * esContext.screenHeight/2) - 2; i+=3) {
    CuAssertIntEquals ( tc, 0x00, screenPixels[i]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+1]);
    CuAssertIntEquals ( tc, 0xff, screenPixels[i+2]);
  }

  gre_scene_add_so ( sceneHandle, quadRed );

  CuAssertIntEquals ( tc, 1, gre_info_num_vshaders ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 1, gre_info_num_fshaders ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 1, gre_info_num_programs ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 1, gre_info_num_buffers ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 0, gre_info_num_textures ( sceneHandle ) );

  gre_scene_snapshot ( sceneHandle, 0, 0, esContext.screenWidth / 2, esContext.screenHeight, screenPixels );
  for (i = 0; i < (3 * esContext.screenWidth * esContext.screenHeight/2) - 2; i+=3) {
    CuAssertIntEquals ( tc, 0xff, screenPixels[i]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+1]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+2]);
  }

  gre_scene_snapshot ( sceneHandle, esContext.screenWidth / 2, 0, esContext.screenWidth / 2, esContext.screenHeight, screenPixels );
  for (i = 0; i < (3 * esContext.screenWidth * esContext.screenHeight/2) - 2; i+=3) {
    CuAssertIntEquals ( tc, 0x00, screenPixels[i]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+1]);
    CuAssertIntEquals ( tc, 0xff, screenPixels[i+2]);
  }

  gre_scene_stop ( sceneHandle );

  gre_scene_free ( sceneHandle );

  gre_so_free ( quadRed );
  gre_so_free ( quadBlue );
  gre_attribute_free (attrBluePos);
  gre_attribute_free (attrRedPos);
  gre_draw_free(drawRed);
  gre_draw_free(drawBlue);
  gre_uniform_free(uniBlueColor);
  gre_uniform_free(uniRedColor);

  gre_tear_down();

  escontext_release(&esContext);
}

void testDrawElements1(CuTest* tc) {

  gre_initialize();

  unsigned short vIndicesRed[] = {
    0, 1, 2, 3, 4, 5
  };
  GREHandle quadRedIndices = gre_create_attribute(vIndicesRed, TYPE_UNSIGNED_SHORT, sizeof(unsigned short)*6, 1, 1);
  GREHandle quadRedDraw = gre_create_draw_elements(DRAW_TRIANGLES, 0, 6, quadRedIndices);
  GREHandle quadRed = gre_create_so(16, vShaderPosition, fShaderColor, quadRedDraw);  
  GLfloat vVerticesRed[] = {
    -1.0f,  -1.0f, 
    0.0f, -1.0f, 
    -1.0f, 1.0f, 

    -1.0f, 1.0f, 
    0.0f, -1.0f,
    0.0f, 1.0f,
  };
  GREHandle quadRedPos = gre_create_attribute(vVerticesRed, TYPE_FLOAT, sizeof(float)*12, 2, 1);
  gre_so_attribute_set ( quadRed, "position", quadRedPos);
  float colorRed[] = {
    1.0f, 0.0f, 0.0f, 1.0f
  };
  GREHandle quadRedColor = gre_create_uniform(colorRed);  
  gre_so_uniform_set ( quadRed, "color", quadRedColor );


  unsigned short vIndicesBlue[] = {
    0, 1, 2, 3, 4, 5
  };
  GREHandle quadBlueIndices = gre_create_attribute(vIndicesBlue, TYPE_UNSIGNED_SHORT, sizeof(unsigned short)*6, 1, 0);
  GREHandle quadBlueDraw = gre_create_draw_elements(DRAW_TRIANGLES, 0, 6, quadBlueIndices);
  GREHandle quadBlue = gre_create_so(16, vShaderPosition, fShaderColor, quadBlueDraw);  
  GLfloat vVerticesBlue[] = {
    0.0f, -1.0f,
    1.0f, -1.0f,
    0.0f, 1.0f,
    0.0f, 1.0f, 
    1.0f, -1.0f,
    1.0f, 1.0f,
  };
  GREHandle quadBluePos = gre_create_attribute(vVerticesBlue, TYPE_FLOAT, sizeof(float)*12, 2, 0);
  gre_so_attribute_set ( quadBlue, "position", quadBluePos);
  float colorBlue[] = {
    0.0f, 0.0f, 1.0f, 1.0f
  };
  GREHandle quadBlueColor = gre_create_uniform(colorBlue);  
  gre_so_uniform_set ( quadBlue, "color", quadBlueColor );


  GREHandle sceneHandle = gre_scene_alloc();
  gre_scene_add_so (sceneHandle, quadRed );  
  gre_scene_add_so ( sceneHandle, quadBlue );
  

  ESContext esContext;
  escontext_init (&esContext, NULL);

  RenderConfig renderConfig;
  renderConfig.framerate = FRAMERATE_30;
  renderConfig.esContext = &esContext;

  gre_scene_start ( sceneHandle, &renderConfig );

  CuAssertIntEquals ( tc, 1, gre_info_num_vshaders ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 1, gre_info_num_fshaders ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 1, gre_info_num_programs ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 2, gre_info_num_buffers ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 0, gre_info_num_textures ( sceneHandle ) );


  char screenPixels[ 3 * esContext.screenWidth * esContext.screenHeight / 2 ];
   
  int i;
  gre_scene_snapshot ( sceneHandle, 0, 0, esContext.screenWidth / 2, esContext.screenHeight, screenPixels );
  for (i = 0; i < (3 * esContext.screenWidth * esContext.screenHeight/2) - 2; i+=3) {
    CuAssertIntEquals ( tc, 0xff, screenPixels[i]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+1]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+2]);
  }

  gre_scene_snapshot ( sceneHandle, esContext.screenWidth / 2, 0, esContext.screenWidth / 2, esContext.screenHeight, screenPixels );

  for (i = 0; i < (3 * esContext.screenWidth * esContext.screenHeight/2) - 2; i+=3) {
    CuAssertIntEquals ( tc, 0x00, screenPixels[i]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+1]);
    CuAssertIntEquals ( tc, 0xff, screenPixels[i+2]);
  }

  gre_scene_remove_so ( sceneHandle, quadRed );

  CuAssertIntEquals ( tc, 1, gre_info_num_vshaders ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 1, gre_info_num_fshaders ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 1, gre_info_num_programs ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 0, gre_info_num_buffers ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 0, gre_info_num_textures ( sceneHandle ) );

  gre_scene_snapshot ( sceneHandle, 0, 0, esContext.screenWidth / 2, esContext.screenHeight, screenPixels );
  for (i = 0; i < (3 * esContext.screenWidth * esContext.screenHeight/2) - 2; i+=3) {
    CuAssertIntEquals ( tc, 0x00, screenPixels[i]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+1]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+2]);
  }

  gre_scene_snapshot ( sceneHandle, esContext.screenWidth / 2, 0, esContext.screenWidth / 2, esContext.screenHeight, screenPixels );
  for (i = 0; i < (3 * esContext.screenWidth * esContext.screenHeight/2) - 2; i+=3) {
    CuAssertIntEquals ( tc, 0x00, screenPixels[i]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+1]);
    CuAssertIntEquals ( tc, 0xff, screenPixels[i+2]);
  }

  gre_scene_add_so ( sceneHandle, quadRed );

  CuAssertIntEquals ( tc, 1, gre_info_num_vshaders ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 1, gre_info_num_fshaders ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 1, gre_info_num_programs ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 2, gre_info_num_buffers ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 0, gre_info_num_textures ( sceneHandle ) );

  gre_scene_snapshot ( sceneHandle, 0, 0, esContext.screenWidth / 2, esContext.screenHeight, screenPixels );
  for (i = 0; i < (3 * esContext.screenWidth * esContext.screenHeight/2) - 2; i+=3) {
    CuAssertIntEquals ( tc, 0xff, screenPixels[i]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+1]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+2]);
  }

  gre_scene_snapshot ( sceneHandle, esContext.screenWidth / 2, 0, esContext.screenWidth / 2, esContext.screenHeight, screenPixels );
  for (i = 0; i < (3 * esContext.screenWidth * esContext.screenHeight/2) - 2; i+=3) {
    CuAssertIntEquals ( tc, 0x00, screenPixels[i]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+1]);
    CuAssertIntEquals ( tc, 0xff, screenPixels[i+2]);
  }

  gre_scene_stop ( sceneHandle );

  gre_scene_free ( sceneHandle );

  gre_so_free ( quadRed );
  gre_so_free ( quadBlue );

  gre_attribute_free (quadRedIndices);
  gre_attribute_free (quadRedPos);
  gre_draw_free(quadRedDraw);
  gre_uniform_free (quadRedColor);


  gre_attribute_free (quadBlueIndices);
  gre_attribute_free (quadBluePos);
  gre_draw_free(quadBlueDraw);
  gre_uniform_free (quadBlueColor);

  gre_tear_down();

  escontext_release(&esContext);

}

void testDrawElements2(CuTest* tc) {
  gre_initialize();

  unsigned short vIndicesRed[] = {
    0, 1, 2, 3, 4, 5
  };
  GREHandle quadRedIndices = gre_create_attribute(vIndicesRed, TYPE_UNSIGNED_SHORT, sizeof(unsigned short)*6, 1, 1);
  GREHandle quadRedDraw = gre_create_draw_elements(DRAW_TRIANGLES, 0, 6, quadRedIndices);
  GREHandle quadRed = gre_create_so(16, vShaderPosition, fShaderColor, quadRedDraw);
  GLfloat vVerticesRed[] = {
    -1.0f,  -1.0f, 
    0.0f, -1.0f, 
    -1.0f, 1.0f, 

    -1.0f, 1.0f, 
    0.0f, -1.0f,
    0.0f, 1.0f,
  };
  GREHandle quadRedPosition = gre_create_attribute(vVerticesRed, TYPE_FLOAT, sizeof(float)*12, 2, 1);
  gre_so_attribute_set ( quadRed, "position", quadRedPosition);
  float colorRed[] = {
    1.0f, 0.0f, 0.0f, 1.0f
  };
  GREHandle quadRedColor = gre_create_uniform(colorRed);
  gre_so_uniform_set ( quadRed, "color", quadRedColor );


  GREHandle quadBlue = gre_create_so(16, vShaderPosition, fShaderColor, quadRedDraw);
  GLfloat vVerticesBlue[] = {
    0.0f, -1.0f,
    1.0f, -1.0f,
    0.0f, 1.0f,
    0.0f, 1.0f, 
    1.0f, -1.0f,
    1.0f, 1.0f,
  };
  GREHandle quadBluePosition = gre_create_attribute(vVerticesBlue, TYPE_FLOAT, sizeof(float)*12, 2, 0);
  gre_so_attribute_set ( quadBlue, "position", quadBluePosition);
  float colorBlue[] = {
    0.0f, 0.0f, 1.0f, 1.0f
  };
  GREHandle quadBlueColor = gre_create_uniform(colorBlue);
  gre_so_uniform_set ( quadBlue, "color", quadBlueColor );


  GREHandle sceneHandle = gre_scene_alloc();
  gre_scene_add_so (sceneHandle, quadRed );  
  gre_scene_add_so ( sceneHandle, quadBlue );
  

  ESContext esContext;
  escontext_init (&esContext, NULL);

  RenderConfig renderConfig;
  renderConfig.framerate = FRAMERATE_30;
  renderConfig.esContext = &esContext;

  gre_scene_start ( sceneHandle, &renderConfig );

  CuAssertIntEquals ( tc, 1, gre_info_num_vshaders ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 1, gre_info_num_fshaders ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 1, gre_info_num_programs ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 2, gre_info_num_buffers ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 0, gre_info_num_textures ( sceneHandle ) );

  
  char screenPixels[ 3 * esContext.screenWidth * esContext.screenHeight / 2 ];
   
  int i;
  gre_scene_snapshot ( sceneHandle, 0, 0, esContext.screenWidth / 2, esContext.screenHeight, screenPixels );


  for (i = 0; i < (3 * esContext.screenWidth * esContext.screenHeight/2) - 2; i+=3) {
    CuAssertIntEquals ( tc, 0xff, screenPixels[i]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+1]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+2]);
  }

  gre_scene_snapshot ( sceneHandle, esContext.screenWidth / 2, 0, esContext.screenWidth / 2, esContext.screenHeight, screenPixels );
  for (i = 0; i < (3 * esContext.screenWidth * esContext.screenHeight/2) - 2; i+=3) {
    CuAssertIntEquals ( tc, 0x00, screenPixels[i]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+1]);
    CuAssertIntEquals ( tc, 0xff, screenPixels[i+2]);
  }

  gre_scene_remove_so ( sceneHandle, quadRed );

  CuAssertIntEquals ( tc, 1, gre_info_num_vshaders ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 1, gre_info_num_fshaders ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 1, gre_info_num_programs ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 1, gre_info_num_buffers ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 0, gre_info_num_textures ( sceneHandle ) );

  gre_scene_snapshot ( sceneHandle, 0, 0, esContext.screenWidth / 2, esContext.screenHeight, screenPixels );
  for (i = 0; i < (3 * esContext.screenWidth * esContext.screenHeight/2) - 2; i+=3) {
    CuAssertIntEquals ( tc, 0x00, screenPixels[i]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+1]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+2]);
  }

  gre_scene_snapshot ( sceneHandle, esContext.screenWidth / 2, 0, esContext.screenWidth / 2, esContext.screenHeight, screenPixels );
  for (i = 0; i < (3 * esContext.screenWidth * esContext.screenHeight/2) - 2; i+=3) {
    CuAssertIntEquals ( tc, 0x00, screenPixels[i]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+1]);
    CuAssertIntEquals ( tc, 0xff, screenPixels[i+2]);
  }

  gre_scene_add_so ( sceneHandle, quadRed );

  CuAssertIntEquals ( tc, 1, gre_info_num_vshaders ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 1, gre_info_num_fshaders ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 1, gre_info_num_programs ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 2, gre_info_num_buffers ( sceneHandle ) );  
  CuAssertIntEquals ( tc, 0, gre_info_num_textures ( sceneHandle ) );

  gre_scene_snapshot ( sceneHandle, 0, 0, esContext.screenWidth / 2, esContext.screenHeight, screenPixels );
  for (i = 0; i < (3 * esContext.screenWidth * esContext.screenHeight/2) - 2; i+=3) {
    CuAssertIntEquals ( tc, 0xff, screenPixels[i]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+1]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+2]);
  }

  gre_scene_snapshot ( sceneHandle, esContext.screenWidth / 2, 0, esContext.screenWidth / 2, esContext.screenHeight, screenPixels );
  for (i = 0; i < (3 * esContext.screenWidth * esContext.screenHeight/2) - 2; i+=3) {
    CuAssertIntEquals ( tc, 0x00, screenPixels[i]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+1]);
    CuAssertIntEquals ( tc, 0xff, screenPixels[i+2]);
  }

  gre_scene_stop ( sceneHandle );

  gre_scene_free ( sceneHandle );
  
  gre_so_free ( quadRed );
  gre_attribute_free (quadRedPosition);
  gre_attribute_free (quadRedIndices);
  gre_draw_free (quadRedDraw);
  gre_uniform_free (quadRedColor);

  gre_so_free ( quadBlue );
  gre_attribute_free (quadBluePosition);
  gre_uniform_free (quadBlueColor);
  
  gre_tear_down();

  escontext_release(&esContext);

}

void testBufferUsage(CuTest* tc) {

  gre_initialize();

  ////////// RED
  unsigned short vIndicesRed[] = {
    0, 1, 2, 3, 4, 5
  };
  GREHandle quadRedIndices = gre_create_attribute(vIndicesRed, TYPE_UNSIGNED_SHORT, sizeof(unsigned short)*6, 1, 0);
  GREHandle quadRedDraw = gre_create_draw_elements(DRAW_TRIANGLES, 0, 6, quadRedIndices);
  GREHandle quadRed = gre_create_so(16, vShaderPosition, fShaderColor, quadRedDraw);
  GLfloat vVerticesRed[] = {
    -1.0f, 0.0f,
    0.0f, 0.0f,
    -1.0f, 1.0f,

    -1.0f, 1.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
  };
  GREHandle quadRedPosition = gre_create_attribute(vVerticesRed, TYPE_FLOAT, sizeof(float)*12, 2, 0);
  gre_so_attribute_set ( quadRed, "position", quadRedPosition);
  float colorRed[] = {
    1.0f, 0.0f, 0.0f, 1.0f
  };
  GREHandle quadRedColor = gre_create_uniform(colorRed);  
  gre_so_uniform_set ( quadRed, "color", quadRedColor );


  ////////// GREEN

  GREHandle quadGreenDraw = gre_create_draw_arrays(DRAW_TRIANGLES, 0, 6);
  GREHandle quadGreen = gre_create_so(16, vShaderPosition, fShaderColor, quadGreenDraw);
  GLfloat vVerticesGreen[] = {
    -1.0f,  -1.0f, 
    0.0f, -1.0f, 
    -1.0f, 0.0f, 

    -1.0f, 0.0f, 
    0.0f, -1.0f,
    0.0f, 0.0f,
  };
  GREHandle quadGreenPosition = gre_create_attribute(vVerticesGreen, TYPE_FLOAT, sizeof(float)*12, 2, 1);
  gre_so_attribute_set ( quadGreen, "position", quadGreenPosition);  
  float colorGreen[] = {
    0.0f, 1.0f, 0.0f, 1.0f
  };
  GREHandle quadGreenColor = gre_create_uniform(colorGreen);
  gre_so_uniform_set ( quadGreen, "color", quadGreenColor );


  ////////// BLUE

  unsigned short indicesBlueData[] = {
    0,1,2,3,4,5,
  };
  GREHandle quadBlueIndices = gre_create_attribute(indicesBlueData, TYPE_UNSIGNED_SHORT, sizeof(unsigned short)*6, 1, 1);
  GREHandle quadBlueDraw = gre_create_draw_elements(DRAW_TRIANGLES, 0, 6, quadBlueIndices);
  GREHandle quadBlue = gre_create_so(16, vShaderPosition, fShaderColor, quadBlueDraw);
  GLfloat vVerticesBlue[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,

    0.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
  };
  GREHandle quadBluePosition = gre_create_attribute(vVerticesBlue, TYPE_FLOAT, sizeof(float)*12, 2, 1);
  gre_so_attribute_set ( quadBlue, "position", quadBluePosition);  
  float colorBlue[] = {
    0.0f, 0.0f, 1.0f, 1.0f
  };
  GREHandle quadBlueColor = gre_create_uniform(colorBlue);
  gre_so_uniform_set ( quadBlue, "color", quadBlueColor );


  ////////// YELLOW

  GREHandle quadYellow = gre_create_so(16, vShaderPosition, fShaderColor, quadBlueDraw);
  GLfloat vVerticesYellow[] = {
    0.0f, -1.0f,
    1.0f, -1.0f,
    0.0f, 0.0f,

    0.0f, 0.0f,
    1.0f, -1.0f,
    1.0f, 0.0f,
  };
  GREHandle quadYellowPosition = gre_create_attribute(vVerticesYellow, TYPE_FLOAT, sizeof(float)*12, 2, 1);
  gre_so_attribute_set ( quadYellow, "position", quadYellowPosition);
  float colorYellow[] = {
    1.0f, 1.0f, 0.0f, 1.0f
  };
  GREHandle quadYellowColor = gre_create_uniform(colorYellow);
  gre_so_uniform_set ( quadYellow, "color", quadYellowColor );

  // START
  GREHandle sceneHandle = gre_scene_alloc();

  gre_scene_add_so (sceneHandle, quadRed );  

  ESContext esContext;
  escontext_init (&esContext, NULL);

  RenderConfig renderConfig;
  renderConfig.framerate = FRAMERATE_30;
  renderConfig.esContext = &esContext;

  gre_scene_start ( sceneHandle, &renderConfig );    

  // TESTS
  CuAssertIntEquals ( tc, 0, gre_info_num_buffers ( sceneHandle ) );  

  gre_scene_add_so ( sceneHandle, quadGreen );
  CuAssertIntEquals ( tc, 1, gre_info_num_buffers ( sceneHandle ) );    

  gre_scene_add_so ( sceneHandle, quadBlue );

  CuAssertIntEquals ( tc, 3, gre_info_num_buffers ( sceneHandle ) );  

  gre_scene_add_so ( sceneHandle, quadYellow );
    
  CuAssertIntEquals ( tc, 4, gre_info_num_buffers ( sceneHandle ) );  

  gre_scene_remove_so ( sceneHandle, quadYellow );

  CuAssertIntEquals ( tc, 3, gre_info_num_buffers ( sceneHandle ) );  

  gre_scene_add_so ( sceneHandle, quadYellow );

  CuAssertIntEquals ( tc, 4, gre_info_num_buffers ( sceneHandle ) );  

  gre_scene_remove_so ( sceneHandle, quadBlue );

  CuAssertIntEquals ( tc, 3, gre_info_num_buffers ( sceneHandle ) );  
  
  gre_scene_add_so ( sceneHandle, quadBlue );

  CuAssertIntEquals ( tc, 4, gre_info_num_buffers ( sceneHandle ) );  

  char screenPixels[ 3 * esContext.screenHeight * esContext.screenWidth / 4];
   
  int i;
  gre_scene_snapshot ( sceneHandle, 0, esContext.screenHeight/2, esContext.screenWidth / 2, esContext.screenHeight / 2, screenPixels );
  for (i = 0; i < (3 * esContext.screenWidth * esContext.screenHeight/4) - 2; i+=3) {
    CuAssertIntEquals ( tc, 0xff, screenPixels[i]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+1]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+2]);
  }

  gre_scene_snapshot ( sceneHandle, 0, 0, esContext.screenWidth / 2, esContext.screenHeight / 2, screenPixels );
  for (i = 0; i < (3 * esContext.screenWidth * esContext.screenHeight/4) - 2; i+=3) {
    CuAssertIntEquals ( tc, 0x00, screenPixels[i]);
    CuAssertIntEquals ( tc, 0xff, screenPixels[i+1]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+2]);
  }

  gre_scene_snapshot ( sceneHandle, esContext.screenWidth/2, esContext.screenHeight/2, esContext.screenWidth / 2, esContext.screenHeight / 2, screenPixels );
  for (i = 0; i < (3 * esContext.screenWidth * esContext.screenHeight/4) - 2; i+=3) {
    CuAssertIntEquals ( tc, 0x00, screenPixels[i]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+1]);
    CuAssertIntEquals ( tc, 0xff, screenPixels[i+2]);
  }

  gre_scene_snapshot ( sceneHandle, esContext.screenWidth/2, 0, esContext.screenWidth / 2, esContext.screenHeight / 2, screenPixels );
  for (i = 0; i < (3 * esContext.screenWidth * esContext.screenHeight/4) - 2; i+=3) {
    CuAssertIntEquals ( tc, 0xff, screenPixels[i]);
    CuAssertIntEquals ( tc, 0xff, screenPixels[i+1]);
    CuAssertIntEquals ( tc, 0x00, screenPixels[i+2]);
  }

  gre_scene_remove_so ( sceneHandle, quadRed );

  CuAssertIntEquals ( tc, 4, gre_info_num_buffers ( sceneHandle ) );

  gre_scene_remove_so ( sceneHandle, quadGreen );

  CuAssertIntEquals ( tc, 3, gre_info_num_buffers ( sceneHandle ) );

  gre_scene_stop ( sceneHandle );

  gre_scene_free ( sceneHandle );

  gre_so_free ( quadRed );
  gre_attribute_free (quadRedPosition);
  gre_attribute_free (quadRedIndices);
  gre_uniform_free (quadRedColor);
  gre_draw_free (quadRedDraw);

  gre_so_free ( quadGreen );
  gre_attribute_free(quadGreenPosition);
  gre_uniform_free(quadGreenColor);
  gre_draw_free(quadGreenDraw);

  gre_so_free ( quadBlue );
  gre_attribute_free (quadBluePosition);
  gre_attribute_free (quadBlueIndices);
  gre_uniform_free (quadBlueColor);
  gre_draw_free (quadBlueDraw);

  gre_so_free ( quadYellow );
  gre_attribute_free (quadYellowPosition);
  gre_uniform_free(quadYellowColor);

  gre_tear_down();

  escontext_release(&esContext);
}


CuSuite* GREGetSuite() { 
  CuSuite* suite = CuSuiteNew();
  
  SUITE_ADD_TEST(suite, testDrawArrays);  
  
  SUITE_ADD_TEST(suite, testDrawElements1);  
  SUITE_ADD_TEST(suite, testDrawElements2);  
  SUITE_ADD_TEST(suite, testBufferUsage);
  
  return suite;
}

void RunAllTests(void) 
{
  CuString *output = CuStringNew();
  CuSuite* suite = GREGetSuite();  
  
  CuSuiteRun(suite);
  CuSuiteSummary(suite, output);
  CuSuiteDetails(suite, output);
  printf("%s\n", output->buffer);
  CuStringDelete(output);
  CuSuiteDelete(suite);
}

int main(int argc, const char* argv[])
{
  RunAllTests();
  return 0;
}

