#include <stdlib.h>
#include "../core/api.h"
#include "../shaders/specular_shader.h"
#include "specular_scenes.h"

scene_t *specular_drone_scene(void) {
    const char *mesh = "assets/drone/drone.obj";
    specular_material_t material = {
        {1, 1, 1, 1},
        {1, 1, 1},
        1,
        0,
        "assets/drone/diffuse.tga",
        "assets/drone/specular.tga",
        "assets/drone/glossiness.tga",
        NULL,
        "assets/drone/occlusion.tga",
        "assets/drone/emissive.tga",
        0,
        0,
    };
    vec4_t background = vec4_new(0.196f, 0.196f, 0.196f, 1);
    const char *env_name = "papermill";
    model_t **models = NULL;
    scene_t *scene;

    mat4_t translation = mat4_translate(0, -79.181f, -4.447f);
    mat4_t rotation = mat4_rotate_y(TO_RADIANS(180));
    mat4_t scale = mat4_scale(0.0028f, 0.0028f, 0.0028f);
    mat4_t root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    model_t *model = specular_create_model(mesh, root, material, env_name);
    darray_push(models, model);

    scene = (scene_t*)malloc(sizeof(scene_t));
    scene->background = background;
    scene->skybox     = NULL;
    scene->models     = models;

    return scene;
}
