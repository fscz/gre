#ifndef gre_gre_h
#define gre_gre_h

/****************************************************************************
 * Copyright (C) 2015 by Fabian Schuetz                                     *
 *                                                                          *
 * This file is part of Box.                                                *
 *                                                                          *
 *   Box is free software: you can redistribute it and/or modify it         *
 *   under the terms of the GNU Lesser General Public License as published  *
 *   by the Free Software Foundation, either version 3 of the License, or   *
 *   (at your option) any later version.                                    *
 *                                                                          *
 *   Box is distributed in the hope that it will be useful,                 *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU Lesser General Public License for more details.                    *
 *                                                                          *
 *   You should have received a copy of the GNU Lesser General Public       *
 *   License along with Box.  If not, see <http://www.gnu.org/licenses/>.   *
 ****************************************************************************/

/**
 * @file gre.h
 * @author Fabian Schuetz
 * @date 28 Mai 2015
 * @brief gre - generic render engine api include file
 *
 * gre is a lightweight 3d capable opengl es 2.0 render engine for the 
 * Raspberry Pi.
 *
 * It abstracts away many details of the render pipeline by providing
 * a single concise api file <gre.h>. In particular you never have to
 * call a gl-function, create or delete framebuffers, vertex buffers 
 * and so on. 
 *
 * gre operates on the following core types:
 * : Attribute - An opengl Attribute. gre also offers automatic buffering.
 * : Uniform - An opengl Uniform. A Uniform can be a texture, any float/int based data.
 * : Shader - A Shader is a char*
 * : SceneObject - A SceneObject is a { 
 *                        vertexShader:Shader, 
 *                        fragmentShader:Shader, 
 *                        uniforms:map<char*,Uniform>, 
 *                        attributes:map<char*,Attribute>,
 *                        drawMethod:Draw }
 * : Draw - A value holder struct that details how a SceneObject is drawn (GL_TRIANGLES,etc)
 * : Scene - A Scene is essentially a list of SceneObjects and some internal state values
 * 
 * The above types are normally created by calling one the gre_[type]_alloc or gre_create_[type]
 * functions. The latter are convenience functions that are composed of simpler functions.
 * gre_create_[type] and gre_[type]_alloc functions return a GREHandle that is subsequently used
 * to refer to the object. 
 *
 * This requires some explanation: OpenGL defines different types of objects the lifecycle
 * of wich needs to be managed by the programmer. Such objects include Framebuffers, Renderbuffers,
 * Vertex Buffers, Textures and so on. Such objects generally need to be created, setup and filled 
 * with data. In many cases data needs to be transferred from main memory to the graphics card. 
 * This is especially true for Vertex Buffers and Textures. A programmer thus has to manage data
 * on both the graphics card and the main memory. gre abstracts the management of graphics memory
 * away by allocating the respective GL objects automatically, when they are needed. 
 *
 * For example when rendering a mesh, vertex data is usually a large buffer of positions, normals and
 * so on. This data is automatically transferred to the graphics card, if a SceneObject is setup with
 * 1. An Attribute object in its attributes map and 2. it's vertex shader requires the attribute. 
 * By passing the buffered=true flag on the Attribute object it is indicated that data should not be 
 * transferred on every draw call, rather it is transferred once and stored on the graphics card.
 * 
 * Following this idea of buffering data on the graphics card, it is desirable to reuse buffers. For different
 * Scene Objects. It is quite common for example to reuse the same mesh data for multiple objects in a scene.
 * gre allows that by using the same GREHandle for the given Attribute on multiple Scene Objects. 
 *
 * The same mechanism works for shaders and Textures. 
 * 
 * gre performs reference counting for users of Vertex Buffers and Textures and so on. If the number of users falls
 * to 0, the object is also automatically cleared from the graphics card.
 * 
 */


#include <stdio.h>
#include <stdlib.h>

#include "GLES2/gl2.h"
#include "escontext.h"

#include "types.h"
#include "util.h"

/*******************************************************************************
 * DEFINITIONS
/******************************************************************************/
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

/*******************************************************************************
 * INIT/TEAR-DOWN
/******************************************************************************/
int gre_initialize();
int gre_tear_down();

/*******************************************************************************
 * SCENE
/******************************************************************************/
GREHandle gre_scene_alloc();
void gre_scene_free(GREHandle sceneHandle);
int gre_scene_add_so(GREHandle sceneHandle, GREHandle soHandle);
int gre_scene_remove_so(GREHandle sceneHandle, GREHandle soHandle);
int gre_scene_start(GREHandle sceneHandle, size_t fps, void (*updateFunc) (void* userData, unsigned long long), void* userData);
int gre_scene_stop(GREHandle sceneHandle);
void gre_scene_snapshot(GREHandle sceneHandle, size_t x, size_t y, size_t width, size_t height, char* buffer);

/*******************************************************************************
 * ATTRIBUTE
/******************************************************************************/
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

/*******************************************************************************
 * UNIFORM
/******************************************************************************/
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

/*******************************************************************************
 * DRAW
/******************************************************************************/
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

/*******************************************************************************
 * SCENEOBJECT
/******************************************************************************/
GREHandle gre_so_alloc();
void gre_so_free(GREHandle soHandle);
void gre_so_attribute_set(GREHandle soHandle, const char* name, GREHandle attributeHandle);
GREHandle gre_so_attribute_get(GREHandle soHandle, const char* name);
void gre_so_uniform_set(GREHandle soHandle, const char* name, GREHandle uniformHandle);
GREHandle gre_so_uniform_get(GREHandle soHandle, const char* name);
void gre_so_vshader_set(GREHandle soHandle, const char* vShader);
void gre_so_fshader_set(GREHandle soHandle, const char* fShader);

/*******************************************************************************
 * CREATORS
/******************************************************************************/
GREHandle gre_create_attribute(void* data, GREenum dataType, size_t size, size_t numComponents, uint useBuffer);
GREHandle gre_create_uniform(void* data);
GREHandle gre_create_uniform_texture(void* data, GREenum textureFormat, GREenum textureType, size_t width, size_t height, GREenum sampleMode);
GREHandle gre_create_uniform_texture_handle(uint textureHandle, GREenum sampleMode);
GREHandle gre_create_draw_arrays(GREenum primitiveType, size_t start, size_t count);
GREHandle gre_create_draw_elements(GREenum primitiveType, size_t start, size_t count, GREHandle attributeHandle);
GREHandle gre_create_so(uint stage_bit, const char* vShader, const char* fShader, GREHandle drawHandle);

/*******************************************************************************
 * UTILITY
/******************************************************************************/
GREHandle gre_gbuffer_get();
GREHandle gre_lightbuffer_get();
int gre_get_screen_size(size_t* width, size_t* height);
size_t gre_info_num_vshaders(GREHandle sceneHandle);
size_t gre_info_num_fshaders(GREHandle sceneHandle);
size_t gre_info_num_programs(GREHandle sceneHandle);
size_t gre_info_num_buffers(GREHandle sceneHandle);
size_t gre_info_num_textures(GREHandle sceneHandle);


#endif
