#include "lambert_scenes.h"
#include <stdio.h>
#include <stdlib.h>
#include "../core/apis.h"
#include "../shaders/lambert_shader.h"

scene_t *lambert_craftsman_scene(void) {
    const char *anvil_obj = "assets/craftsman/anvil.obj";
    const char *floor_obj = "assets/craftsman/floor.obj";
    const char *hammer_obj = "assets/craftsman/hammer.obj";
    const char *hotiron_obj = "assets/craftsman/hotiron.obj";
    const char *shoulderpad0_obj = "assets/craftsman/shoulderpad0.obj";
    const char *shoulderpad1_obj = "assets/craftsman/shoulderpad1.obj";
    const char *smith_obj = "assets/craftsman/smith.obj";
    const char *anvil_diffuse_tga = "assets/craftsman/anvil_diffuse.tga";
    const char *floor_diffuse_tga = "assets/craftsman/floor_diffuse.tga";
    const char *smith_diffuse_tga = "assets/craftsman/smith_diffuse.tga";
    const char *smith_emission_tga = "assets/craftsman/smith_emission.tga";
    mat4_t anvil_transform = {{
        {  0.936571f,   0.000000f,   0.000000f,   0.000000f},
        {  0.000000f,   0.000000f,   0.936571f,  -0.000000f},
        {  0.000000f,  -0.936571f,   0.000000f,  23.366537f},
        {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
    }};
    mat4_t floor_transform = {{
        {  1.259828f,   0.000000f,   0.000000f,   1.668093f},
        {  0.000000f,   0.000000f,   0.885767f,  -0.000000f},
        {  0.000000f,  -1.259828f,   0.000000f,  10.833580f},
        {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
    }};
    mat4_t hammer_transform = {{
        {  0.324427f,   0.217420f,   0.920584f,  11.052060f},
        { -0.852411f,   0.489096f,   0.184888f,  78.562383f},
        { -0.410056f,  -0.844697f,   0.344006f,  -8.390521f},
        {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
    }};
    mat4_t hotiron_transform = {{
        {  0.840193f,  -0.540788f,  -0.040288f, -34.668739f},
        { -0.102606f,  -0.085585f,  -0.991035f,  17.130267f},
        {  0.532492f,   0.836793f,  -0.127395f,  56.477256f},
        {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
    }};
    mat4_t shoulderpad0_transform = {{
        {  1.000000f,   0.000000f,   0.000000f,   0.000000f},
        {  0.000000f,   0.000000f,   1.000000f,   0.000000f},
        {  0.000000f,  -1.000000f,   0.000000f,  -0.000000f},
        {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
    }};
    mat4_t shoulderpad1_transform = {{
        {  0.954568f,  -0.168932f,  -0.248769f,  10.987287f},
        {  0.171253f,  -0.392511f,   0.908502f,   8.632905f},
        { -0.243870f,  -0.936879f,  -0.335766f,  10.008259f},
        {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
    }};
    mat4_t smith_transform = {{
        {  1.000000f,   0.000000f,   0.000000f,   0.000000f},
        {  0.000000f,   0.000000f,   1.000000f,   0.000000f},
        {  0.000000f,  -1.000000f,   0.000000f,   0.000000f},
        {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
    }};
    scene_t *scene;
    lambert_material_t material;
    model_t **models = NULL;
    model_t *model;
    mat4_t scale, translation, root;
    mat4_t transform;

    translation = mat4_translate(-1.668f, -27.061f, -10.834f);
    scale = mat4_scale(0.031f, 0.031f, 0.031f);
    root = mat4_mul_mat4(scale, translation);

    material.ambient_factor = vec4_new(0, 0, 0, 1);
    material.diffuse_factor = vec4_new(1, 1, 1, 1);
    material.emission_factor = vec4_new(0, 0, 0, 1);
    material.diffuse_texture = NULL;
    material.emission_texture = NULL;

    material.diffuse_texture = anvil_diffuse_tga;
    transform = mat4_mul_mat4(root, anvil_transform);
    model = lambert_create_model(anvil_obj, transform, material);
    darray_push(models, model);

    material.diffuse_texture = floor_diffuse_tga;
    transform = mat4_mul_mat4(root, floor_transform);
    model = lambert_create_model(floor_obj, transform, material);
    darray_push(models, model);

    material.emission_factor = vec4_new(1, 1, 1, 1);
    material.diffuse_texture = smith_diffuse_tga;
    material.emission_texture = smith_emission_tga;

    transform = mat4_mul_mat4(root, hammer_transform);
    model = lambert_create_model(hammer_obj, transform, material);
    darray_push(models, model);

    transform = mat4_mul_mat4(root, hotiron_transform);
    model = lambert_create_model(hotiron_obj, transform, material);
    darray_push(models, model);

    transform = mat4_mul_mat4(root, shoulderpad0_transform);
    model = lambert_create_model(shoulderpad0_obj, transform, material);
    darray_push(models, model);

    transform = mat4_mul_mat4(root, shoulderpad1_transform);
    model = lambert_create_model(shoulderpad1_obj, transform, material);
    darray_push(models, model);

    transform = mat4_mul_mat4(root, smith_transform);
    model = lambert_create_model(smith_obj, transform, material);
    darray_push(models, model);

    scene = (scene_t*)malloc(sizeof(scene_t));
    scene->background = vec4_new(0, 0, 0, 1);
    scene->models     = models;

    return scene;
}
