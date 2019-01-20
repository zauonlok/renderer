#include "blinn_scenes.h"
#include <stdio.h>
#include <stdlib.h>
#include "../core/apis.h"
#include "../shaders/blinn_shader.h"

static scene_t *create_scene(
        int num_meshes, const char *mesh_names[], const char *emission_names[],
        const char *diffuse_names[], const char *specular_name[],
        mat4_t transforms[], vec4_t background) {
    vec4_t enabled = vec4_new(1, 1, 1, 1);
    vec4_t disabled = vec4_new(0, 0, 0, 1);
    model_t **models = NULL;
    scene_t *scene;
    int i;

    for (i = 0; i < num_meshes; i++) {
        blinn_material_t material;
        model_t *model;

        material.ambient_factor = disabled;
        material.emission_factor = emission_names[i] ? enabled : disabled;
        material.diffuse_factor = diffuse_names[i] ? enabled : disabled;
        material.specular_factor = specular_name[i] ? enabled : disabled;
        material.shininess = 32;
        material.emission_texture = emission_names[i];
        material.diffuse_texture = diffuse_names[i];
        material.specular_texture = specular_name[i];

        model = blinn_create_model(mesh_names[i], transforms[i], material);
        darray_push(models, model);
    }

    scene = (scene_t*)malloc(sizeof(scene_t));
    scene->background = background;
    scene->models     = models;

    return scene;
}

scene_t *blinn_centaur_scene(void) {
    const char *mesh_names[] = {
        "assets/centaur/body.obj",
        "assets/centaur/flame.obj",
        "assets/centaur/gas.obj",
    };
    const char *emission_names[] = {
        "assets/centaur/body_emission.tga",
        "assets/centaur/flame_emission.tga",
        NULL,
    };
    const char *diffuse_names[] = {
        "assets/centaur/body_diffuse.tga",
        "assets/centaur/flame_diffuse.tga",
        "assets/centaur/gas_diffuse.tga",
    };
    const char *specular_name[] = {
        "assets/centaur/body_specular.tga",
        NULL,
        "assets/centaur/gas_specular.tga",
    };
    vec4_t background = vec4_new(0.368f, 0.392f, 0.337f, 1);
    mat4_t transforms[3];
    mat4_t scale, rotation, translation, transform;
    scene_t *scene;
    int num_meshes = 3;
    int i;

    translation = mat4_translate(0.154f, -7.579f, -30.749f);
    rotation = mat4_rotate_x(TO_RADIANS(-90));
    rotation = mat4_mul_mat4(mat4_rotate_y(TO_RADIANS(-90)), rotation);
    scale = mat4_scale(0.016f, 0.016f, 0.016f);
    transform = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    for (i = 0; i < num_meshes; i++) {
        transforms[i] = transform;
    }

    scene = create_scene(num_meshes, mesh_names, emission_names, diffuse_names,
                         specular_name, transforms, background);
    return scene;
}
