#include <stdlib.h>
#include "../core/api.h"
#include "../shaders/specular_shader.h"
#include "specular_scenes.h"

scene_t *specular_crab_scene(void) {
    const char *mesh = "assets/crab/crab.obj";
    specular_material_t material = {
        {1, 1, 1},
        "assets/crab/diffuse.tga",
        {1, 1, 1},
        "assets/crab/specular.tga",
        1,
        "assets/crab/glossiness.tga",
        "assets/crab/normal.tga",
        NULL,
        NULL,
    };
    vec4_t background = vec4_new(0.196f, 0.196f, 0.196f, 1);
    const char *env_name = "papermill";
    model_t **models = NULL;
    model_t *model;
    scene_t *scene;
    mat4_t scale, rotation, root;

    rotation = mat4_rotate_y(TO_RADIANS(135));
    scale = mat4_scale(0.167f, 0.167f, 0.167f);
    root = mat4_mul_mat4(scale, rotation);
    model = specular_create_model(mesh, root, material, env_name);
    darray_push(models, model);

    scene = (scene_t*)malloc(sizeof(scene_t));
    scene->background = background;
    scene->models     = models;

    return scene;
}
