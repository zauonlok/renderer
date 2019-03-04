#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../core/api.h"
#include "../shaders/skinned_shader.h"
#include "skinned_scenes.h"

scene_t *skinned_assassin_scene(void) {
    const char *meshes[] = {
        "assets/assassin/body.obj",
        "assets/assassin/face.obj",
        "assets/assassin/hair.obj",
        "assets/assassin/weapon.obj",
    };
    skinned_material_t materials[] = {
        {{1, 1, 1, 1}, "assets/assassin/body.tga", 0, 0},
        {{1, 1, 1, 1}, "assets/assassin/face.tga", 0, 0},
        {{1, 1, 1, 1}, "assets/assassin/hair.tga", 0, 0},
        {{1, 1, 1, 1}, "assets/assassin/weapon.tga", 0, 0},
    };
    vec4_t background = vec4_new(0.690f, 0.576f, 0.576f, 1);
    model_t **models = NULL;
    model_t *model;
    scene_t *scene;
    mat4_t scale, translation, root;
    int num_meshes = ARRAY_SIZE(meshes);
    int i;

    assert(ARRAY_SIZE(materials) == num_meshes);

    translation = mat4_translate(0, -125.815f, 18.898f);
    scale = mat4_scale(0.0038f, 0.0038f, 0.0038f);
    root = mat4_mul_mat4(scale, translation);
    for (i = 0; i < num_meshes; i++) {
        model = skinned_create_model(meshes[i], root, materials[i]);
        darray_push(models, model);
    }

    scene = (scene_t*)malloc(sizeof(scene_t));
    scene->background = background;
    scene->models     = models;
    scene->userdata   = skeleton_load("assets/assassin/assassin.ani");

    return scene;
}

scene_t *skinned_junkrat_scene(void) {
    skinned_material_t materials[] = {
        {{1, 1, 1, 1}, "assets/junkrat/upper.tga", 0, 0},
        {{1, 1, 1, 1}, "assets/junkrat/lower.tga", 0, 0},
        {{1, 1, 1, 1}, "assets/junkrat/head.tga", 0, 0},
        {{1, 1, 1, 1}, "assets/junkrat/back.tga", 0, 0},
    };
    int mesh2material[63] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 2, 0, 1, 0, 2, 1, 0,
        0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2,
    };
    vec4_t background = vec4_new(0.196f, 0.196f, 0.196f, 1);
    model_t **models = NULL;
    model_t *model;
    scene_t *scene;
    mat4_t scale, translation, root;
    int num_meshes = ARRAY_SIZE(mesh2material);
    int i;

    translation = mat4_translate(3.735f, -382.993f, 57.980f);
    scale = mat4_scale(0.0013f, 0.0013f, 0.0013f);
    root = mat4_mul_mat4(scale, translation);
    for (i = 0; i < num_meshes; i++) {
        int material_index = mesh2material[i];
        skinned_material_t material = materials[material_index];
        const char *obj_template = "assets/junkrat/junkrat%d.obj";
        char obj_filename[64];
        sprintf(obj_filename, obj_template, i);
        model = skinned_create_model(obj_filename, root, material);
        darray_push(models, model);
    }

    scene = (scene_t*)malloc(sizeof(scene_t));
    scene->background = background;
    scene->models     = models;
    scene->userdata   = skeleton_load("assets/junkrat/junkrat.ani");

    return scene;
}

scene_t *skinned_kgirls_scene(void) {
    const char *meshes[] = {
        "assets/kgirls/body.obj",
        "assets/kgirls/face.obj",
        "assets/kgirls/hair.obj",
        "assets/kgirls/pupils.obj",
    };
    skinned_material_t material = {
        {1, 1, 1, 1}, "assets/kgirls/kgirls.tga", 0, 0
    };
    vec4_t background = vec4_new(0.196f, 0.196f, 0.196f, 1);
    model_t **models = NULL;
    model_t *model;
    scene_t *scene;
    mat4_t scale, rotation, translation, root;
    int num_meshes = ARRAY_SIZE(meshes);
    int i;

    translation = mat4_translate(0, -4.937f, -96.547f);
    rotation = mat4_rotate_x(TO_RADIANS(-90));
    rotation = mat4_mul_mat4(mat4_rotate_y(TO_RADIANS(90)), rotation);
    scale = mat4_scale(0.005f, 0.005f, 0.005f);
    root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    for (i = 0; i < num_meshes; i++) {
        model = skinned_create_model(meshes[i], root, material);
        darray_push(models, model);
    }

    scene = (scene_t*)malloc(sizeof(scene_t));
    scene->background = background;
    scene->models     = models;
    scene->userdata   = skeleton_load("assets/kgirls/kgirls.ani");

    return scene;
}
