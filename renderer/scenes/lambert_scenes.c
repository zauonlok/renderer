#include <assert.h>
#include <stdlib.h>
#include "../core/api.h"
#include "../shaders/lambert_shader.h"
#include "lambert_scenes.h"

scene_t *lambert_craftsman_scene(void) {
    const char *meshes[] = {
        "assets/craftsman/anvil.obj",
        "assets/craftsman/floor.obj",
        "assets/craftsman/hammer.obj",
        "assets/craftsman/hotiron.obj",
        "assets/craftsman/shoulderpad0.obj",
        "assets/craftsman/shoulderpad1.obj",
        "assets/craftsman/smith.obj",
    };
    lambert_material_t materials[] = {
        {
            0.5f,
            NULL,
            "assets/craftsman/anvil_diffuse.tga",
        },
        {
            0.5f,
            NULL,
            "assets/craftsman/floor_diffuse.tga",
        },
        {
            0.5f,
            "assets/craftsman/smith_emission.tga",
            "assets/craftsman/smith_diffuse.tga",
        },
        {
            0.5f,
            "assets/craftsman/smith_emission.tga",
            "assets/craftsman/smith_diffuse.tga",
        },
        {
            0.5f,
            "assets/craftsman/smith_emission.tga",
            "assets/craftsman/smith_diffuse.tga",
        },
        {
            0.5f,
            "assets/craftsman/smith_emission.tga",
            "assets/craftsman/smith_diffuse.tga",
        },
        {
            0.5f,
            "assets/craftsman/smith_emission.tga",
            "assets/craftsman/smith_diffuse.tga",
        },
    };
    mat4_t transforms[] = {
        {{
            {  0.936571f,   0.000000f,   0.000000f,   0.000000f},
            {  0.000000f,   0.000000f,   0.936571f,   0.000000f},
            {  0.000000f,  -0.936571f,   0.000000f,  23.366537f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            {  1.259828f,   0.000000f,   0.000000f,   1.668093f},
            {  0.000000f,   0.000000f,   0.885767f,   0.000000f},
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
            {  0.000000f,  -1.000000f,   0.000000f,   0.000000f},
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
    model_t **models = NULL;
    model_t *model;
    scene_t *scene;
    mat4_t scale, translation, root;
    int num_meshes = ARRAY_LENGTH(meshes);
    int i;

    assert(ARRAY_LENGTH(materials) == num_meshes);
    assert(ARRAY_LENGTH(transforms) == num_meshes);

    translation = mat4_translate(-1.668f, -27.061f, -10.834f);
    scale = mat4_scale(0.016f, 0.016f, 0.016f);
    root = mat4_mul_mat4(scale, translation);
    for (i = 0; i < num_meshes; i++) {
        mat4_t transform = mat4_mul_mat4(root, transforms[i]);
        model = lambert_create_model(meshes[i], transform, materials[i]);
        darray_push(models, model);
    }

    scene = (scene_t*)malloc(sizeof(scene_t));
    scene->background = background;
    scene->models     = models;

    return scene;
}
