#ifndef SCENE_H
#define SCENE_H

#include "geometry.h"
#include "graphics.h"
#include "mesh.h"
#include "skeleton.h"
#include "texture.h"

typedef struct {
    float ambient_strength;
    float punctual_strength;
} lightdata_t;

typedef struct {
    framebuffer_t *framebuffer;
    texture_t *shadow_map;
} shadowdata_t;

typedef struct {
    float frame_time;
    float delta_time;
    vec3_t light_dir;
    vec3_t camera_pos;
    mat4_t light_view_matrix;
    mat4_t light_proj_matrix;
    mat4_t camera_view_matrix;
    mat4_t camera_proj_matrix;
    float ambient_strength;
    float punctual_strength;
    texture_t *shadow_map;
} framedata_t;

typedef struct {
    int opaque;
    float distance;
} sortdata_t;

typedef struct model {
    mesh_t *mesh;
    skeleton_t *skeleton;
    program_t *program;
    mat4_t transform;
    sortdata_t sortdata;
    int node_index;
    void (*draw)(struct model *model, framebuffer_t *framebuffer,
                 int shadow_pass);
    void (*update)(struct model *model, framedata_t *framedata);
    void (*release)(struct model *model);
} model_t;

typedef struct {
    vec4_t background;
    model_t *skybox;
    model_t **models;
    lightdata_t lightdata;
    shadowdata_t shadowdata;
} scene_t;

scene_t *scene_create(
    vec4_t background, model_t *skybox, model_t **models,
    float ambient_strength, float punctual_strength, int with_shadow);
void scene_release(scene_t *scene);
void scene_draw(scene_t *scene, framebuffer_t *framebuffer,
                framedata_t *framedata);

#endif
