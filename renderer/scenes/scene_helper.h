#ifndef SCENE_HELPER_H
#define SCENE_HELPER_H

#include "../core/api.h"

scene_t *helper_load_blinn_scene(const char *filename, mat4_t root_transform);
scene_t *helper_load_pbrm_scene(const char *filename, mat4_t root_transform);
scene_t *helper_load_pbrs_scene(const char *filename, mat4_t root_transform);

#endif
