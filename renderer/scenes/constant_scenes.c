#include "constant_scenes.h"
#include <stdio.h>
#include <stdlib.h>
#include "../core/apis.h"
#include "../shaders/constant_shader.h"

static scene_t *create_scene(
        int num_meshes, const char *mesh_names[], const char *emission_names[],
        mat4_t transforms[], vec4_t background) {
    vec4_t enabled = vec4_new(1, 1, 1, 1);
    vec4_t disabled = vec4_new(0, 0, 0, 1);
    model_t **models = NULL;
    scene_t *scene;
    int i;

    for (i = 0; i < num_meshes; i++) {
        constant_material_t material;
        model_t *model;

        material.ambient_factor = disabled;
        material.emission_factor = emission_names[i] ? enabled : disabled;
        material.emission_texture = emission_names[i];

        model = constant_create_model(mesh_names[i], transforms[i], material);
        darray_push(models, model);
    }

    scene = (scene_t*)malloc(sizeof(scene_t));
    scene->background = background;
    scene->models     = models;

    return scene;
}

scene_t *constant_mccree_scene(void) {
    mat4_t transforms[8] = {
        {{
            {+0.186340f, +0.000000f, +0.000000f, -0.006200f},
            {+0.000000f, +0.000000f, +0.186340f, +0.131450f},
            {+0.000000f, -0.186340f, +0.000000f, -0.010720f},
            {+0.000000f, +0.000000f, +0.000000f, +1.000000f},
        }},
        {{
            {+0.239850f, +0.008740f, +0.004060f, -0.014310f},
            {-0.020360f, +0.034120f, +0.052410f, +2.016510f},
            {+0.008460f, -0.165570f, +0.011010f, -0.055260f},
            {+0.000000f, +0.000000f, +0.000000f, +1.000000f},
        }},
        {{
            {+0.287440f, -0.019260f, +0.000000f, -0.008370f},
            {-0.007360f, -0.108460f, -0.010090f, +2.267050f},
            {-0.017800f, -0.266190f, +0.004110f, -0.083880f},
            {+0.000000f, +0.000000f, +0.000000f, +1.000000f},
        }},
        {{
            {-0.039080f, -0.003570f, +0.033020f, +0.279880f},
            {-0.009790f, -0.000520f, -0.132230f, +1.397640f},
            {-0.003590f, +0.040280f, +0.001210f, +0.006560f},
            {+0.000000f, +0.000000f, +0.000000f, +1.000000f},
        }},
        {{
            {-0.039130f, -0.006760f, +0.025830f, +0.252460f},
            {-0.006030f, -0.009210f, -0.131150f, +1.364990f},
            {-0.008250f, +0.038800f, -0.026640f, -0.108820f},
            {+0.000000f, +0.000000f, +0.000000f, +1.000000f},
        }},
        {{
            {-0.039130f, -0.006760f, +0.025830f, +0.182500f},
            {-0.006030f, -0.009210f, -0.131150f, +1.340350f},
            {-0.008250f, +0.038800f, -0.026640f, -0.178540f},
            {+0.000000f, +0.000000f, +0.000000f, +1.000000f},
        }},
        {{
            {-0.019630f, -0.037890f, +0.016320f, +0.498480f},
            {-0.013760f, -0.058690f, -0.014030f, +1.926990f},
            {-0.031050f, +0.049970f, -0.004100f, +0.389470f},
            {+0.000000f, +0.000000f, +0.000000f, +1.000000f},
        }},
        {{
            {-0.000780f, +0.023700f, +0.000570f, -0.440940f},
            {-0.004270f, -0.000700f, +0.023330f, +0.955610f},
            {-0.023320f, -0.000670f, -0.004290f, +0.163730f},
            {+0.000000f, +0.000000f, +0.000000f, +1.000000f},
        }},
    };
    constant_material_t materials[28] = {
        {{0.614f, 0.396f, 0.464f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
        {{0.689f, 0.441f, 0.516f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
        {{0.736f, 0.471f, 0.552f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
        {{0.430f, 0.241f, 0.296f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
        {{0.347f, 0.855f, 1.000f, 1}, {0.167f, 0.411f, 0.481f, 1}, NULL},
        {{0.399f, 0.522f, 0.397f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
        {{0.850f, 0.160f, 0.086f, 1}, {0.812f, 0.097f, 0.000f, 1}, NULL},
        {{1.000f, 0.423f, 0.170f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
        {{0.193f, 0.374f, 0.333f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
        {{1.000f, 1.000f, 1.000f, 1}, {0.463f, 0.463f, 0.463f, 1}, NULL},
        {{0.317f, 0.158f, 0.229f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
        {{0.425f, 0.198f, 0.233f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
        {{0.779f, 0.310f, 0.263f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
        {{0.964f, 0.744f, 0.622f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
        {{0.641f, 0.252f, 0.234f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
        {{0.973f, 0.801f, 0.705f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
        {{0.938f, 0.814f, 0.687f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
        {{0.748f, 0.596f, 0.418f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
        {{0.831f, 0.618f, 0.525f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
        {{0.694f, 0.510f, 0.427f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
        {{0.336f, 0.149f, 0.205f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
        {{0.445f, 0.195f, 0.270f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
        {{0.444f, 0.187f, 0.207f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
        {{0.927f, 0.483f, 0.498f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
        {{0.815f, 0.403f, 0.418f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
        {{0.827f, 0.430f, 0.449f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
        {{0.542f, 0.222f, 0.247f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
        {{1.000f, 0.808f, 0.571f, 1}, {0.000f, 0.000f, 0.000f, 1}, NULL},
    };
    int mesh2transform[46] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 3, 3, 3, 4, 4,
        4, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7,
    };
    int mesh2material[46] = {
        17, 13, 18, 19, 20, 21,  3, 11, 22, 23, 24, 25,  0,  4, 26,  2,
         1,  6,  9, 27, 12, 13, 14, 15, 16,  3, 11, 10,  4,  1, 10,  4,
         1, 10,  4,  1,  5,  6,  7,  8,  9,  0,  1,  2,  3,  4,
    };
    vec4_t background = vec4_new(0.847f, 0.949f, 0.898f, 1);
    model_t **models = NULL;
    scene_t *scene;
    mat4_t scale, translation, root;
    int num_meshes = 46;
    int i;

    translation = mat4_translate(0.111f, -1.479f, 0.033f);
    scale = mat4_scale(0.299f, 0.299f, 0.299f);
    root = mat4_mul_mat4(scale, translation);
    for (i = 0; i < num_meshes; i++) {
        int transform_index = mesh2transform[i];
        int material_index = mesh2material[i];
        mat4_t transform = mat4_mul_mat4(root, transforms[transform_index]);
        constant_material_t material = materials[material_index];
        const char *obj_template = "assets/mccree/part%d.obj";
        char obj_filename[128];
        model_t *model;
        sprintf(obj_filename, obj_template, i);
        model = constant_create_model(obj_filename, transform, material);
        darray_push(models, model);
    }

    scene = (scene_t*)malloc(sizeof(scene_t));
    scene->background = background;
    scene->models     = models;

    return scene;
}

scene_t *constant_elfgirl_scene(void) {
    const char *mesh_names[] = {
        "assets/elfgirl/face0.obj",
        "assets/elfgirl/face1.obj",
        "assets/elfgirl/body0.obj",
        "assets/elfgirl/body1.obj",
        "assets/elfgirl/body2.obj",
        "assets/elfgirl/hair.obj",
        "assets/elfgirl/base.obj",
    };
    const char *emission_names[] = {
        "assets/elfgirl/face.tga",
        "assets/elfgirl/face.tga",
        "assets/elfgirl/body.tga",
        "assets/elfgirl/body.tga",
        "assets/elfgirl/body.tga",
        "assets/elfgirl/hair.tga",
        "assets/elfgirl/base.tga",
    };
    vec4_t background = vec4_new(0.333f, 0.333f, 0.333f, 1);
    mat4_t transforms[7];
    mat4_t scale, rotation, translation, transform;
    scene_t *scene;
    int num_meshes = 7;
    int i;

    translation = mat4_translate(2.449f, -2.472f, -20.907f);
    rotation = mat4_rotate_x(TO_RADIANS(-90));
    scale = mat4_scale(0.023f, 0.023f, 0.023f);
    transform = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    for (i = 0; i < num_meshes; i++) {
        transforms[i] = transform;
    }

    scene = create_scene(num_meshes, mesh_names, emission_names,
                         transforms, background);
    return scene;
}
