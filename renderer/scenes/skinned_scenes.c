#include <assert.h>
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
    mat4_t scale, rotation, translation, root;
    int num_meshes = ARRAY_SIZE(meshes);
    int i;

    assert(ARRAY_SIZE(materials) == num_meshes);

    translation = mat4_translate(0, -125.815f, 18.898f);
    rotation = mat4_rotate_z(TO_RADIANS(-90));
    scale = mat4_scale(0.0038f, 0.0038f, 0.0038f);
    root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
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
