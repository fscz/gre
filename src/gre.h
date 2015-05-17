#ifndef gre_gre_h
#define gre_gre_h

#include <stdio.h>
#include <stdlib.h>

#include "GLES2/gl2.h"
#include "escontext.h"

#include "types.h"
#include "util.h"

typedef size_t GREHandle;
typedef size_t GREenum;

#define TEXTURE_HANDLE_GBUFFER                        0xffff                    
#define TEXTURE_HANDLE_LIGHTBUFFER                    0xfffe
#define TEXTURE_HANDLE_DEPTHBUFFER                    0xfffd

#define TEXTURE_FORMAT_ALPHA                          0x1906
#define TEXTURE_FORMAT_RGB                            0x1907
#define TEXTURE_FORMAT_RGBA                           0x1908
#define TEXTURE_FORMAT_LUMINANCE                      0x1909
#define TEXTURE_FORMAT_LUMINANCE_ALPHA                0x190A

#define TEXTURE_TYPE_UNSIGNED_BYTE                    0x1401
#define TEXTURE_TYPE_UNSIGNED_SHORT_4_4_4_4           0x8033
#define TEXTURE_TYPE_UNSIGNED_SHORT_5_5_5_1           0x8034
#define TEXTURE_TYPE_UNSIGNED_SHORT_5_6_5             0x8363

#define DRAW_POINTS                                   0x0000
#define DRAW_LINES                                    0x0001
#define DRAW_LINE_LOOP                                0x0002
#define DRAW_LINE_STRIP                               0x0003
#define DRAW_TRIANGLES                                0x0004
#define DRAW_TRIANGLE_STRIP                           0x0005
#define DRAW_TRIANGLE_FAN                             0x0006

#define SAMPLE_MODE_REPEAT                            0x2901
#define SAMPLE_MODE_CLAMP_TO_EDGE                     0x812F
#define SAMPLE_MODE_MIRRORED_REPEAT                   0x8370

#define TYPE_BYTE                                     0x1400
#define TYPE_UNSIGNED_BYTE                            0x1401
#define TYPE_SHORT                                    0x1402
#define TYPE_UNSIGNED_SHORT                           0x1403
#define TYPE_FLOAT                                    0x1406
#define TYPE_FIXED                                    0x1400


#define FRAMERATE_30 33333333
#define FRAMERATE_60 16666666


int gre_initialize();
int gre_tear_down();
int gre_get_screen_size(size_t* width, size_t* height);


GREHandle gre_scene_alloc();
void gre_scene_free(GREHandle sceneHandle);
int gre_scene_add_so(GREHandle sceneHandle, GREHandle soHandle);
int gre_scene_remove_so(GREHandle sceneHandle, GREHandle soHandle);
int gre_scene_set_camera(GREHandle sceneHandle, Camera* camera);
int gre_scene_start(GREHandle sceneHandle, size_t fps, void (*updateFunc) (void* userData, unsigned long long), void* userData);
int gre_scene_stop(GREHandle sceneHandle);
void gre_scene_snapshot(GREHandle sceneHandle, size_t x, size_t y, size_t width, size_t height, char* buffer);


GREHandle gre_attribute_alloc();
void gre_attribute_free(GREHandle attributeHandle);
void gre_attribute_data_set(GREHandle attributeHandle, void* data);
void* gre_attribute_data_get(GREHandle attributeHandle);
void gre_attribute_data_type_set(GREHandle attributeHandle, GREenum dataType);
GREenum gre_attribute_data_type_get(GREHandle attributeHandle);
void gre_attribute_size_set(GREHandle attributeHandle, size_t size);
size_t gre_attribute_size_get(GREHandle attributeHandle);
void gre_attribute_num_components_set(GREHandle attributeHandle, size_t numComponents);
size_t gre_attribute_num_components_get(GREHandle attributeHandle);
void gre_attribute_use_buffer_set(GREHandle attributeHandle, uint useBuffer);
uint gre_attribute_use_buffer_get(GREHandle attributeHandle);


GREHandle gre_uniform_alloc();
void gre_uniform_free(GREHandle uniformHandle);
void gre_uniform_data_set(GREHandle uniformHandle, void* data);
void* gre_uniform_data_get(GREHandle uniformHandle);
void gre_uniform_texture_format_set(GREHandle uniformHandle, GREenum format);
GREenum gre_uniform_texture_format_get(GREHandle uniformHandle);
void gre_uniform_texture_type_set(GREHandle uniformHandle, GREenum type);
GREenum gre_uniform_texture_type_get(GREHandle uniformHandle);
void gre_uniform_width_set(GREHandle uniformHandle, size_t width);
size_t gre_uniform_width_get(GREHandle uniformHandle);
void gre_uniform_height_set(GREHandle uniformHandle, size_t height);
size_t gre_uniform_height_get(GREHandle uniformHandle);
void gre_uniform_texture_handle_set(GREHandle uniformHandle, uint textureHandle);
uint gre_uniform_texture_handle_get(GREHandle uniformHandle);
void gre_uniform_sample_mode_set(GREHandle uniformHandle, GREenum sampleMode);
GREenum gre_uniform_sample_mode_get(GREHandle uniformHandle);


GREHandle gre_draw_alloc();
void gre_draw_free(GREHandle drawHandle);
void gre_draw_type_set(GREHandle drawHandle, GREenum type);
GREenum gre_draw_type_get(GREHandle drawHandle);
void gre_draw_count_set(GREHandle drawHandle, size_t count);
size_t gre_draw_count_get(GREHandle drawHandle);
void gre_draw_start_set(GREHandle drawHandle, size_t start);
size_t gre_draw_start_get(GREHandle drawHandle);
void gre_draw_indices_set(GREHandle drawHandle, GREHandle attributeHandle);
GREHandle gre_draw_indices_get(GREHandle drawHandle);


GREHandle gre_so_alloc();
void gre_so_free(GREHandle soHandle);
void gre_so_attribute_set(GREHandle soHandle, const char* name, GREHandle attributeHandle);
GREHandle gre_so_attribute_get(GREHandle soHandle, const char* name);
void gre_so_uniform_set(GREHandle soHandle, const char* name, GREHandle uniformHandle);
GREHandle gre_so_uniform_get(GREHandle soHandle, const char* name);
void gre_so_vshader_set(GREHandle soHandle, const char* vShader);
void gre_so_fshader_set(GREHandle soHandle, const char* fShader);


GREHandle gre_create_attribute(void* data, GREenum dataType, size_t size, size_t numComponents, uint useBuffer);
GREHandle gre_create_uniform(void* data);
GREHandle gre_create_uniform_texture(void* data, GREenum textureFormat, GREenum textureType, size_t width, size_t height, GREenum sampleMode);
GREHandle gre_create_uniform_texture_handle(uint textureHandle, GREenum sampleMode);
GREHandle gre_create_draw_arrays(GREenum primitiveType, size_t start, size_t count);
GREHandle gre_create_draw_elements(GREenum primitiveType, size_t start, size_t count, GREHandle attributeHandle);
GREHandle gre_create_so(uint stage_bit, const char* vShader, const char* fShader, GREHandle drawHandle);


GREHandle gre_gbuffer_get();
GREHandle gre_lightbuffer_get();


size_t gre_info_num_vshaders(GREHandle sceneHandle);
size_t gre_info_num_fshaders(GREHandle sceneHandle);
size_t gre_info_num_programs(GREHandle sceneHandle);
size_t gre_info_num_buffers(GREHandle sceneHandle);
size_t gre_info_num_textures(GREHandle sceneHandle);


#endif
