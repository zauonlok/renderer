#include <assert.h>
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

scene_t *metalness_helmet2_scene(void) {
    const char *mesh = "assets/helmet2/helmet.obj";
    metalness_material_t material = {
        {1, 1, 1},
        "assets/helmet2/basecolor.tga",
        1,
        "assets/helmet2/metallic.tga",
        1,
        "assets/helmet2/roughness.tga",
        "assets/helmet2/normal.tga",
        "assets/helmet2/occlusion.tga",
        "assets/helmet2/emissive.tga",
    };
    vec4_t background = vec4_new(0.224f, 0.294f, 0.294f, 1);
    const char *env_name = "papermill";
    model_t **models = NULL;
    scene_t *scene;

    mat4_t root = mat4_scale(0.5f, 0.5f, 0.5f);
    model_t *model = metalness_create_model(mesh, root, material, env_name);
    darray_push(models, model);

    scene = (scene_t*)malloc(sizeof(scene_t));
    scene->background = background;
    scene->models     = models;

    return scene;
}

scene_t *metalness_ponycar_scene(void) {
    const char *meshes[] = {
        "assets/ponycar/body.obj",
        "assets/ponycar/interior.obj",
        "assets/ponycar/windows.obj",
    };
    metalness_material_t materials[] = {
        {
            {1, 1, 1},
            "assets/ponycar/body_basecolor.tga",
            1,
            "assets/ponycar/body_metallic.tga",
            1,
            "assets/ponycar/body_roughness.tga",
            "assets/ponycar/body_normal.tga",
            NULL,
            "assets/ponycar/body_emissive.tga",
        },
        {
            {1, 1, 1},
            "assets/ponycar/interior_basecolor.tga",
            1,
            "assets/ponycar/interior_metallic.tga",
            1,
            "assets/ponycar/interior_roughness.tga",
            "assets/ponycar/interior_normal.tga",
            NULL,
            NULL,
        },
        {
            {1, 1, 1},
            "assets/ponycar/body_basecolor.tga",
            1,
            "assets/ponycar/body_metallic.tga",
            1,
            "assets/ponycar/body_roughness.tga",
            "assets/ponycar/body_normal.tga",
            NULL,
            "assets/ponycar/body_emissive.tga",
        },
    };
    vec4_t background = vec4_new(0.196f, 0.196f, 0.196f, 1);
    const char *env_name = "papermill";
    model_t **models = NULL;
    model_t *model;
    scene_t *scene;
    mat4_t scale, rotation, translation, root;
    int num_meshes = ARRAY_SIZE(meshes);
    int i;

    assert(ARRAY_SIZE(materials) == num_meshes);

    translation = mat4_translate(-10.343f, -13.252f, -186.343f);
    rotation = mat4_rotate_x(TO_RADIANS(-90));
    scale = mat4_scale(0.0015f, 0.0015f, 0.0015f);
    root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    for (i = 0; i < num_meshes; i++) {
        model = metalness_create_model(meshes[i], root, materials[i], env_name);
        darray_push(models, model);
    }

    scene = (scene_t*)malloc(sizeof(scene_t));
    scene->background = background;
    scene->models     = models;

    return scene;
}

scene_t *metalness_dieselpunk_scene(void) {
    const char *meshes[] = {
        "assets/dieselpunk/ground.obj",
        "assets/dieselpunk/mech.obj",
        "assets/dieselpunk/yingham.obj",
    };
    metalness_material_t materials[] = {
        {
            {1, 1, 1},
            "assets/dieselpunk/ground_basecolor.tga",
            0,
            NULL,
            1,
            "assets/dieselpunk/ground_roughness.tga",
            NULL,
            NULL,
            NULL,
        },
        {
            {1, 1, 1},
            "assets/dieselpunk/mech_basecolor.tga",
            1,
            "assets/dieselpunk/mech_metallic.tga",
            1,
            "assets/dieselpunk/mech_roughness.tga",
            "assets/dieselpunk/mech_normal.tga",
            NULL,
            NULL,
        },
        {
            {1, 1, 1},
            "assets/dieselpunk/yingham_basecolor.tga",
            0,
            NULL,
            1,
            "assets/dieselpunk/yingham_roughness.tga",
            "assets/dieselpunk/yingham_normal.tga",
            NULL,
            NULL,
        },
    };
    vec4_t background = vec4_new(0.314f, 0.255f, 0.255f, 1);
    const char *env_name = "papermill";
    model_t **models = NULL;
    model_t *model;
    scene_t *scene;
    mat4_t scale, rotation, translation, root;
    int num_meshes = ARRAY_SIZE(meshes);
    int i;

    assert(ARRAY_SIZE(materials) == num_meshes);

    translation = mat4_translate(1.036f, -114.817f, 27.682f);
    rotation = mat4_rotate_y(TO_RADIANS(-90));
    scale = mat4_scale(0.0011f, 0.0011f, 0.0011f);
    root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    for (i = 0; i < num_meshes; i++) {
        model = metalness_create_model(meshes[i], root, materials[i], env_name);
        darray_push(models, model);
    }

    scene = (scene_t*)malloc(sizeof(scene_t));
    scene->background = background;
    scene->models     = models;

    return scene;
}
