#include <pthread.h>
#include "gre.h"
#include "renderman.h"
#include "repeating_task.h"
#include "types.h"
#include "util.h"

#define KEYSIZE 10

HashMap renderTasks;
ESContext* esContext = NULL;

static inline int task_render_setup(void* userData) {
  RenderContext* ctx = (RenderContext*)userData;
  ctx->scene->running = 1;
  int ret = renderman_rendertask_setup(ctx);
  return ret;
}

static inline int task_render_repeat(RepeatInfo* info, void* userData) {
  return renderman_rendertask_repeat(info, (RenderContext*)userData);
}

static inline int task_render_tear_down(void* userData) {
  RenderContext* ctx = (RenderContext*)userData;
  ctx->scene->running = 0;
  int ret = renderman_rendertask_tear_down(ctx);
  return ret;
}

//////////////////////////////////////////////////
///////////// engine
//////////////////////////////////////////////////

int gre_initialize() {

  renderTasks = hashmap_alloc();

  esContext = malloc(sizeof(ESContext));

  escontext_init(esContext, NULL);

  renderman_initialize();  

  return 0;
}

int gre_tear_down() {

  renderman_tear_down();

  escontext_release(esContext);

  free ( esContext );

  esContext = NULL;

  hashmap_free( renderTasks );

  return 0;
}

int gre_get_screen_size(size_t* width, size_t* height) {
  if ( !esContext ) return -1;
  *width = (uint32_t)esContext->screenWidth;
  *height = (uint32_t)esContext->screenHeight;
  return 0;
}

//////////////////////////////////////////////////
///////////// Uniform
//////////////////////////////////////////////////

GREHandle gre_uniform_alloc() {
  Uniform* uniform = malloc(sizeof(Uniform));
  uniform->textureHandle = 0;
  uniform->data = NULL;
  return (GREHandle)uniform;
}
void gre_uniform_free(GREHandle uniformHandle) {
  Uniform* uni = (Uniform*)uniformHandle;
  free ( uni );
}
void gre_uniform_data_set(GREHandle uniformHandle, void* data) {
  Uniform* uni = (Uniform*)uniformHandle;
  uni->data = data;
}
void* gre_uniform_data_get(GREHandle uniformHandle) {
  Uniform* uni = (Uniform*)uniformHandle;
  return uni->data;
}
void gre_uniform_texture_format_set(GREHandle uniformHandle, GREenum format) {
  Uniform* uni = (Uniform*)uniformHandle;
  uni->textureFormat = format;
}
GREenum gre_uniform_texture_format_get(GREHandle uniformHandle) {
  Uniform* uni = (Uniform*)uniformHandle;
  return uni->textureFormat;
}
void gre_uniform_texture_type_set(GREHandle uniformHandle, GREenum type) {
  Uniform* uni = (Uniform*)uniformHandle;
  uni->textureType = type;
}
GREenum gre_uniform_texture_type_get(GREHandle uniformHandle) {
  Uniform* uni = (Uniform*)uniformHandle;
  return uni->textureType;
}
void gre_uniform_width_set(GREHandle uniformHandle, size_t width) {
  Uniform* uni = (Uniform*)uniformHandle;
  uni->width = width;
}
size_t gre_uniform_width_get(GREHandle uniformHandle) {
  Uniform* uni = (Uniform*)uniformHandle;
  return uni->width;
}
void gre_uniform_height_set(GREHandle uniformHandle, size_t height) {
  Uniform* uni = (Uniform*)uniformHandle;
  uni->height = height;
}
size_t gre_uniform_height_get(GREHandle uniformHandle) {
  Uniform* uni = (Uniform*)uniformHandle;
  return uni->height;
}
void gre_uniform_texture_handle_set(GREHandle uniformHandle, uint textureHandle) {
  Uniform* uni = (Uniform*)uniformHandle;
  uni->textureHandle = textureHandle;
}
uint gre_uniform_texture_handle_get(GREHandle uniformHandle) {
  Uniform* uni = (Uniform*)uniformHandle;
  return uni->textureHandle;
}
void gre_uniform_sample_mode_set(GREHandle uniformHandle, GREenum sampleMode) {
  Uniform* uni = (Uniform*)uniformHandle;
  uni->sampleMode = sampleMode;
}
GREenum gre_uniform_sample_mode_get(GREHandle uniformHandle) {
  Uniform* uni = (Uniform*)uniformHandle;
  return uni->sampleMode;
}

//////////////////////////////////////////////////
///////////// Attribute
//////////////////////////////////////////////////

GREHandle gre_attribute_alloc() {
  Attribute* attribute = malloc(sizeof(Attribute));
  attribute->data = NULL;
  attribute->useBuffer = 0;
  return (GREHandle)attribute;
}
void gre_attribute_free(GREHandle attributeHandle) {
  Attribute* attr = (Attribute*)attributeHandle;
  free ( attr );
}
void gre_attribute_data_set(GREHandle attributeHandle, void* data) {
  Attribute* attr = (Attribute*)attributeHandle;
  attr->data = data;
}
void* gre_attribute_data_get(GREHandle attributeHandle) {
  Attribute* attr = (Attribute*)attributeHandle;
  return attr->data;
}
void gre_attribute_data_type_set(GREHandle attributeHandle, GREenum dataType) {
  Attribute* attr = (Attribute*)attributeHandle;
  attr->dataType = dataType;
}
GREenum gre_attribute_data_type_get(GREHandle attributeHandle) {
  Attribute* attr = (Attribute*)attributeHandle;
  return attr->dataType;
}
void gre_attribute_size_set(GREHandle attributeHandle, size_t size) {
  Attribute* attr = (Attribute*)attributeHandle;
  attr->size = size;
}
size_t gre_attribute_size_get(GREHandle attributeHandle) {
  Attribute* attr = (Attribute*)attributeHandle;
  return attr->size;
}
void gre_attribute_num_components_set(GREHandle attributeHandle, size_t numComponents) {
  Attribute* attr = (Attribute*)attributeHandle;
  attr->numComponents = numComponents;
}
size_t gre_attribute_num_components_get(GREHandle attributeHandle) {
  Attribute* attr = (Attribute*)attributeHandle;
  return attr->numComponents;
}
void gre_attribute_use_buffer_set(GREHandle attributeHandle, uint useBuffer) {
  Attribute* attr = (Attribute*)attributeHandle;
  attr->useBuffer = useBuffer;
}
uint gre_attribute_use_buffer_get(GREHandle attributeHandle) {
  Attribute* attr = (Attribute*)attributeHandle;
  return attr->useBuffer;
}

//////////////////////////////////////////////////
///////////// Draw
//////////////////////////////////////////////////

GREHandle gre_draw_alloc() {
  Draw* draw = malloc(sizeof(Draw));
  draw->indices = NULL;
  draw->start = 0;
  draw->count = 0;
  return (GREHandle)draw;
}
void gre_draw_free(GREHandle drawHandle) {
  Draw* draw = (Draw*)drawHandle;
  free ( draw );
}
void gre_draw_type_set(GREHandle drawHandle, GREenum type) {
  Draw* draw = (Draw*)drawHandle;
  draw->type = type;;
}
GREenum gre_draw_type_get(GREHandle drawHandle) {
  Draw* draw = (Draw*)drawHandle;
  return draw->type;
}
void gre_draw_count_set(GREHandle drawHandle, size_t count) {
  Draw* draw = (Draw*)drawHandle;
  draw->count = count;
}
size_t gre_draw_count_get(GREHandle drawHandle) {
  Draw* draw = (Draw*)drawHandle;
  return draw->count;
}
void gre_draw_start_set(GREHandle drawHandle, size_t start) {
  Draw* draw = (Draw*)drawHandle;
  draw->start = start;
}
size_t gre_draw_start_get(GREHandle drawHandle) {
  Draw* draw = (Draw*)drawHandle;
  return draw->start;
}
void gre_draw_indices_set(GREHandle drawHandle, GREHandle attributeHandle) {
  Draw* draw = (Draw*)drawHandle;
  draw->indices = (Attribute*)attributeHandle;
}
GREHandle gre_draw_indices_get(GREHandle drawHandle) {
  Draw* draw = (Draw*)drawHandle;
  return (GREHandle)draw->indices;
}


//////////////////////////////////////////////////
///////////// Scene 
//////////////////////////////////////////////////

GREHandle gre_scene_alloc() {

  Scene* scene = malloc(sizeof(Scene));
  scene->initialObjects = list_alloc();
  scene->running = 0;
  return (GREHandle)scene;
}

void gre_scene_free(GREHandle sceneHandle) {
  Scene* scene = (Scene*)sceneHandle;
  list_free( scene->initialObjects );
  free ( scene );
}

int gre_scene_start(GREHandle sceneHandle, size_t fps, void (*updateFunc) (void* userData, unsigned long long), void* userData) {

  Scene* scene = (Scene*)sceneHandle;
  TaskInfo* taskInfo = malloc(sizeof(TaskInfo));
  
  RenderContext* renderContext = malloc(sizeof(RenderContext));
  renderContext->scene = scene;
  renderContext->esContext = esContext;
  renderContext->updateFunc = updateFunc;
  renderContext->userData = userData;
  taskInfo->userData = renderContext;
  taskInfo->setup = task_render_setup;
  taskInfo->repeat = task_render_repeat;
  taskInfo->tearDown = task_render_tear_down;
  taskInfo->intervalNs = 1000000000 / fps;


  char key[KEYSIZE];
  snprintf(key, KEYSIZE, "%d", sceneHandle);
  hashmap_insert(key, taskInfo, renderTasks);
  return repeating_task_start(taskInfo);
}

int gre_scene_stop(GREHandle sceneHandle) {
  char key[KEYSIZE];
  snprintf(key, KEYSIZE, "%d", sceneHandle);

  TaskInfo* taskInfo = hashmap_find(key, renderTasks);

  if (taskInfo != NULL) {
    repeating_task_stop(taskInfo);
    hashmap_delete(key, renderTasks);
    free (taskInfo);

    return 0;
  }
  else return -1;
}

int gre_scene_add_so(GREHandle sceneHandle, GREHandle soHandle) {
  Scene* scene = (Scene*)sceneHandle;
  SceneObject* so = (SceneObject*)soHandle;

  return renderman_add_so (scene, so);
}

int gre_scene_remove_so(GREHandle sceneHandle, GREHandle soHandle) {
  Scene* scene = (Scene*)sceneHandle;
  SceneObject* so = (SceneObject*)soHandle;

  return renderman_remove_so (scene, so);
}

int gre_scene_set_camera(GREHandle sceneHandle, Camera* camera) {
  return 0;
}

void gre_scene_snapshot(GREHandle sceneHandle, size_t x, size_t y, size_t width, size_t height, char* buffer) {
  Scene* scene = (Scene*)sceneHandle;
  renderman_snapshot( scene, x, y, width, height, buffer );
}

//////////////////////////////////////////////////
///////////// Scene Objects
//////////////////////////////////////////////////

GREHandle gre_so_alloc() {
  SceneObject* so = malloc(sizeof(SceneObject));
  so->attributes = hashmap_alloc();
  so->uniforms = hashmap_alloc();
  so->draw = NULL;
  so->vShader = NULL;
  so->fShader = NULL;
  so->stage = 1&4; // GEOMETRY
  return (GREHandle)so;
}

void gre_so_free( GREHandle soHandle ) {
  SceneObject* so = (SceneObject*)soHandle;
  hashmap_free ( so->attributes );
  hashmap_free ( so->uniforms );
  free ( so );
}

void gre_so_attribute_set(GREHandle soHandle, const char* name, GREHandle attributeHandle) {
  SceneObject* so = (SceneObject*)soHandle;
  Attribute* attr = (Attribute*)attributeHandle;
  hashmap_insert ( name, attr, so->attributes );
}

void gre_so_uniform_set(GREHandle soHandle, const char* name, GREHandle uniformHandle) {
  SceneObject* so = (SceneObject*)soHandle;
  Uniform* uni = (Uniform*)uniformHandle;
  hashmap_insert ( name, uni, so->uniforms );
}

void gre_so_vshader_set(GREHandle soHandle, const char* vshader) {
  SceneObject* so = (SceneObject*)soHandle;
  so->vShader = vshader;
}

void gre_so_fshader_set(GREHandle soHandle, const char* fshader) {
  SceneObject* so = (SceneObject*)soHandle;
  so->fShader = fshader;
}

//////////////////////////////////////////////////
///////////// Creators
//////////////////////////////////////////////////


GREHandle gre_create_attribute(void* data, GREenum dataType, size_t size, size_t numComponents, uint useBuffer) {
  Attribute* attr = (Attribute*)gre_attribute_alloc();
  attr->data = data;
  attr->dataType = dataType;
  attr->size = size;
  attr->numComponents = numComponents;
  attr->useBuffer = useBuffer;
  return (GREHandle)attr;
}
GREHandle gre_create_uniform(void* data) {
  Uniform* uni = (Uniform*)gre_uniform_alloc();
  uni->data = data;
  return (GREHandle)uni;
}
GREHandle gre_create_uniform_texture(void* data, GREenum format, GREenum type, size_t width, size_t height, GREenum sampleMode) {
  Uniform* uni = (Uniform*)gre_uniform_alloc();
  uni->data = data;
  uni->textureFormat = format;
  uni->textureType = type;
  uni->width = width;
  uni->height = height;
  uni->sampleMode = sampleMode;
  return (GREHandle)uni;
}
GREHandle gre_create_uniform_texture_handle(uint textureHandle, GREenum sampleMode) {
  Uniform* uni = (Uniform*)gre_uniform_alloc();
  uni->sampleMode = sampleMode;
  uni->textureHandle = textureHandle;
  return (GREHandle)uni;
}
GREHandle gre_create_draw_arrays(GREenum type, size_t start, size_t count) {
  Draw* draw = (Draw*)gre_draw_alloc();
  draw->type = type;
  draw->count = count;
  draw->start = start;
  return (GREHandle)draw;
}
GREHandle gre_create_draw_elements(GREenum type, size_t start, size_t count, GREHandle attributeHandle) {
  Draw* draw = (Draw*)gre_draw_alloc();
  draw->type = type;
  draw->count = count;
  draw->start = start;
  draw->indices = (Attribute*)attributeHandle;
  return (GREHandle)draw;
}
GREHandle gre_create_so(uint stage_bit, const char* vShader, const char* fShader, GREHandle drawHandle) {
  SceneObject* so = (SceneObject*)gre_so_alloc();
  so->draw = (Draw*)drawHandle;
  so->vShader = vShader;
  so->fShader = fShader;
  so->stage = stage_bit;
  return (GREHandle)so;
}

//////////////////////////////////////////////////
///////////// Info
//////////////////////////////////////////////////

size_t gre_info_num_vshaders(GREHandle sceneHandle) {
  Scene* scene = (Scene*)sceneHandle;
  return renderman_info_num_vshaders ( scene );
}

size_t gre_info_num_fshaders(GREHandle sceneHandle) {
  Scene* scene = (Scene*)sceneHandle;
  return renderman_info_num_fshaders ( scene );
}

size_t gre_info_num_programs(GREHandle sceneHandle) {
  Scene* scene = (Scene*)sceneHandle;
  return renderman_info_num_programs ( scene );
}

size_t gre_info_num_buffers(GREHandle sceneHandle) {
  Scene* scene = (Scene*)sceneHandle;
  return renderman_info_num_buffers ( scene );
}

size_t gre_info_num_textures(GREHandle sceneHandle) {
  Scene* scene = (Scene*)sceneHandle;
  return renderman_info_num_textures ( scene );
}
