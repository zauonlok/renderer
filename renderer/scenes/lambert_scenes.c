#include <stdlib.h>
#include "../core/api.h"
#include "../shaders/lambert_shader.h"
#include "lambert_scenes.h"

static scene_t *create_scene(
        int num_meshes, const char *mesh_names[], const char *emission_names[],
        const char *diffuse_names[], mat4_t transforms[], vec4_t background) {
    vec4_t enabled = vec4_new(1, 1, 1, 1);
    vec4_t disabled = vec4_new(0, 0, 0, 1);
    model_t **models = NULL;
    scene_t *scene;
    int i;

    for (i = 0; i < num_meshes; i++) {
        lambert_material_t material;
        model_t *model;

        material.ambient_factor = disabled;
        material.emission_factor = emission_names[i] ? enabled : disabled;
        material.diffuse_factor = diffuse_names[i] ? enabled : disabled;
        material.emission_texture = emission_names[i];
        material.diffuse_texture = diffuse_names[i];

        model = lambert_create_model(mesh_names[i], transforms[i], material);
        darray_push(models, model);
    }

    scene = (scene_t*)malloc(sizeof(scene_t));
    scene->background = background;
    scene->models     = models;

    return scene;
}

scene_t *lambert_craftsman_scene(void) {
    const char *mesh_names[] = {
        "assets/craftsman/anvil.obj",
        "assets/craftsman/floor.obj",
        "assets/craftsman/hammer.obj",
        "assets/craftsman/hotiron.obj",
        "assets/craftsman/shoulderpad0.obj",
        "assets/craftsman/shoulderpad1.obj",
        "assets/craftsman/smith.obj",
    };
    const char *emission_names[] = {
        NULL,
        NULL,
        "assets/craftsman/smith_emission.tga",
        "assets/craftsman/smith_emission.tga",
        "assets/craftsman/smith_emission.tga",
        "assets/craftsman/smith_emission.tga",
        "assets/craftsman/smith_emission.tga",
    };
    const char *diffuse_names[] = {
        "assets/craftsman/anvil_diffuse.tga",
        "assets/craftsman/floor_diffuse.tga",
        "assets/craftsman/smith_diffuse.tga",
        "assets/craftsman/smith_diffuse.tga",
        "assets/craftsman/smith_diffuse.tga",
        "assets/craftsman/smith_diffuse.tga",
        "assets/craftsman/smith_diffuse.tga",
    };
    mat4_t transforms[] = {
        {{
            {  0.936571f,   0.000000f,   0.000000f,   0.000000f},
            {  0.000000f,   0.000000f,   0.936571f,  -0.000000f},
            {  0.000000f,  -0.936571f,   0.000000f,  23.366537f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            {  1.259828f,   0.000000f,   0.000000f,   1.668093f},
            {  0.000000f,   0.000000f,   0.885767f,  -0.000000f},
            {  0.000000f,  -1.259828f,   0.000000f,  10.833580f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            {  0.324427f,   0.217420f,   0.920584f,  11.052060f},
            { -0.852411f,   0.489096f,   0.184888f,  78.562383f},
            { -0.410056f,  -0.844697f,   0.344006f,  -8.390521f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            {  0.840193f,  -0.540788f,  -0.040288f, -34.668739f},
            { -0.102606f,  -0.085585f,  -0.991035f,  17.130267f},
            {  0.532492f,   0.836793f,  -0.127395f,  56.477256f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            {  1.000000f,   0.000000f,   0.000000f,   0.000000f},
            {  0.000000f,   0.000000f,   1.000000f,   0.000000f},
            {  0.000000f,  -1.000000f,   0.000000f,  -0.000000f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            {  0.954568f,  -0.168932f,  -0.248769f,  10.987287f},
            {  0.171253f,  -0.392511f,   0.908502f,   8.632905f},
            { -0.243870f,  -0.936879f,  -0.335766f,  10.008259f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            {  1.000000f,   0.000000f,   0.000000f,   0.000000f},
            {  0.000000f,   0.000000f,   1.000000f,   0.000000f},
            {  0.000000f,  -1.000000f,   0.000000f,   0.000000f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
    };
    vec4_t background = vec4_new(0.078f, 0.082f, 0.102f, 1);
    mat4_t scale, translation, transform;
    scene_t *scene;
    int num_meshes = 7;
    int i;

    translation = mat4_translate(-1.668f, -27.061f, -10.834f);
    scale = mat4_scale(0.013f, 0.013f, 0.013f);
    transform = mat4_mul_mat4(scale, translation);
    for (i = 0; i < num_meshes; i++) {
        transforms[i] = mat4_mul_mat4(transform, transforms[i]);
    }

    scene = create_scene(num_meshes, mesh_names, emission_names,
                         diffuse_names, transforms, background);
    return scene;
}
