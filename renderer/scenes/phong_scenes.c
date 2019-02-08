#include <stdlib.h>
#include "../core/api.h"
#include "../shaders/phong_shader.h"
#include "phong_scenes.h"

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
        phong_material_t material;
        model_t *model;

        material.ambient_factor = disabled;
        material.emission_factor = emission_names[i] ? enabled : disabled;
        material.diffuse_factor = diffuse_names[i] ? enabled : disabled;
        material.specular_factor = specular_name[i] ? enabled : disabled;
        material.shininess = 32;
        material.emission_texture = emission_names[i];
        material.diffuse_texture = diffuse_names[i];
        material.specular_texture = specular_name[i];

        model = phong_create_model(mesh_names[i], transforms[i], material);
        darray_push(models, model);
    }

    scene = (scene_t*)malloc(sizeof(scene_t));
    scene->background = background;
    scene->models     = models;

    return scene;
}

scene_t *phong_ornitier_scene(void) {
    const char *mesh_names[] = {
        "assets/ornitier/base.obj",
        "assets/ornitier/body.obj",
        "assets/ornitier/coat.obj",
        "assets/ornitier/hands.obj",
        "assets/ornitier/hat.obj",
        "assets/ornitier/legs.obj",
    };
    const char *emission_names[] = {
        "assets/ornitier/base_emission.tga",
        "assets/ornitier/body_emission.tga",
        NULL,
        NULL,
        NULL,
        NULL,
    };
    const char *diffuse_names[] = {
        "assets/ornitier/base_diffuse.tga",
        "assets/ornitier/body_diffuse.tga",
        "assets/ornitier/coat_diffuse.tga",
        "assets/ornitier/hands_diffuse.tga",
        "assets/ornitier/hat_diffuse.tga",
        "assets/ornitier/legs_diffuse.tga",
    };
    const char *specular_name[] = {
        "assets/ornitier/base_specular.tga",
        "assets/ornitier/body_specular.tga",
        "assets/ornitier/coat_specular.tga",
        "assets/ornitier/hands_specular.tga",
        "assets/ornitier/hat_specular.tga",
        "assets/ornitier/legs_specular.tga",
    };
    vec4_t background = vec4_new(0.314f, 0.235f, 0.278f, 1);
    mat4_t transforms[6];
    mat4_t scale, translation, transform;
    scene_t *scene;
    int num_meshes = 6;
    int i;

    translation = mat4_translate(5.863f, 156.991f, 110.488f);
    scale = mat4_scale(0.0017f, 0.0017f, 0.0017f);
    transform = mat4_mul_mat4(scale, translation);
    for (i = 0; i < num_meshes; i++) {
        transforms[i] = transform;
    }

    scene = create_scene(num_meshes, mesh_names, emission_names, diffuse_names,
                         specular_name, transforms, background);
    return scene;
}
