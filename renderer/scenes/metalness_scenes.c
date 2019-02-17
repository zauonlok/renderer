#include <stdlib.h>
#include "../core/api.h"
#include "../shaders/metalness_shader.h"
#include "metalness_scenes.h"

scene_t *metalness_helmet_scene(void) {
    const char *mesh = "assets/helmet/helmet.obj";
    metalness_material_t material = {
        {1, 1, 1},
        "assets/helmet/basecolor.tga",
        1,
        "assets/helmet/metallic.tga",
        1,
        "assets/helmet/roughness.tga",
        NULL,
        "assets/helmet/occlusion.tga",
        "assets/helmet/emissive.tga",
    };
    vec4_t background = vec4_new(0.196f, 0.196f, 0.196f, 1);
    const char *env_name = "papermill";
    model_t **models = NULL;
    scene_t *scene;

    mat4_t rotation = mat4_rotate_x(TO_RADIANS(90));
    mat4_t scale = mat4_scale(0.5f, 0.5f, 0.5f);
    mat4_t root = mat4_mul_mat4(scale, rotation);
    model_t *model = metalness_create_model(mesh, root, material, env_name);
    darray_push(models, model);

    scene = (scene_t*)malloc(sizeof(scene_t));
    scene->background = background;
    scene->models     = models;

    return scene;
}
