#ifndef gre_renderman_h
#define gre_rendemanr_h

#include "escontext.h"
#include "gre.h"
#include "types.h"
#include "repeating_task.h"


int renderman_initialize ();
int renderman_tear_down ();

int renderman_rendertask_setup(RenderContext* ctx);
int renderman_rendertask_repeat(RepeatInfo* info, RenderContext* ctx);
int renderman_rendertask_tear_down(RenderContext* ctx);

int renderman_add_so(Scene* scene, SceneObject* so);
int renderman_remove_so(Scene* scene, SceneObject* so);
void renderman_snapshot(Scene* scene, size_t x, size_t y, size_t width, size_t height, char* buffer);

size_t renderman_info_num_buffers(Scene* scene);
size_t renderman_info_num_textures(Scene* scene);
size_t renderman_info_num_vshaders(Scene* scene);
size_t renderman_info_num_fshaders(Scene* scene);
size_t renderman_info_num_programs(Scene* scene);

#endif
