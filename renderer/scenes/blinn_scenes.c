#include <assert.h>
#include <stdlib.h>
#include "../core/api.h"
#include "../shaders/blinn_shader.h"
#include "blinn_scenes.h"

scene_t *blinn_centaur_scene(void) {
    const char *meshes[] = {
        "assets/centaur/body.obj",
        "assets/centaur/flame.obj",
        "assets/centaur/gas.obj",
    };
    blinn_material_t materials[] = {
        {
            0.5f,
            32,
            "assets/centaur/body_emission.tga",
            "assets/centaur/body_diffuse.tga",
            "assets/centaur/body_specular.tga",
        },
        {
            0.5f,
            32,
            "assets/centaur/flame_emission.tga",
            "assets/centaur/flame_diffuse.tga",
            NULL,
        },
        {
            0.5f,
            32,
            NULL,
            "assets/centaur/gas_diffuse.tga",
            "assets/centaur/gas_specular.tga",
        },
    };
    vec4_t background = vec4_new(0.368f, 0.392f, 0.337f, 1);
    model_t **models = NULL;
    model_t *model;
    scene_t *scene;
    mat4_t scale, rotation, translation, root;
    int num_meshes = ARRAY_LENGTH(meshes);
    int i;

    assert(ARRAY_LENGTH(materials) == num_meshes);

    translation = mat4_translate(0.154f, -7.579f, -30.749f);
    rotation = mat4_rotate_x(TO_RADIANS(-90));
    rotation = mat4_mul_mat4(mat4_rotate_y(TO_RADIANS(-90)), rotation);
    scale = mat4_scale(0.016f, 0.016f, 0.016f);
    root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    for (i = 0; i < num_meshes; i++) {
        model = blinn_create_model(meshes[i], root, materials[i]);
        darray_push(models, model);
    }

    scene = (scene_t*)malloc(sizeof(scene_t));
    scene->background = background;
    scene->models     = models;

    return scene;
}
