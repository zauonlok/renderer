#include <assert.h>
#include <stdlib.h>
#include "../core/api.h"
#include "../shaders/metalness_shader.h"
#include "metalness_scenes.h"

scene_t *metalness_helmet_scene(void) {
    const char *mesh = "assets/helmet/helmet.obj";
    metalness_material_t material = {
        {1, 1, 1, 1},
        1,
        1,
        0,
        "assets/helmet/basecolor.tga",
        "assets/helmet/metallic.tga",
        "assets/helmet/roughness.tga",
        NULL,
        "assets/helmet/occlusion.tga",
        "assets/helmet/emissive.tga",
        0,
        0,
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
    scene->skybox     = NULL;
    scene->models     = models;

    return scene;
}

scene_t *metalness_helmet2_scene(void) {
    const char *meshes[] = {
        "assets/helmet2/glass.obj",
        "assets/helmet2/helmet.obj",
    };
    metalness_material_t materials[] = {
        {
            {0.336f, 0.336f, 0.336f, 0.306f},
            0.729f,
            0.240f,
            0,
            NULL,
            NULL,
            "assets/helmet2/glass_roughness.tga",
            NULL,
            NULL,
            NULL,
            0,
            1,
        },
        {
            {1, 1, 1, 1},
            1,
            1,
            0,
            "assets/helmet2/helmet_basecolor.tga",
            "assets/helmet2/helmet_metallic.tga",
            "assets/helmet2/helmet_roughness.tga",
            "assets/helmet2/helmet_normal.tga",
            "assets/helmet2/helmet_occlusion.tga",
            "assets/helmet2/helmet_emissive.tga",
            0,
            0,
        },
    };
    vec4_t background = vec4_new(0.224f, 0.294f, 0.294f, 1);
    const char *env_name = "papermill";
    model_t **models = NULL;
    model_t *model;
    scene_t *scene;
    mat4_t root;
    int num_meshes = ARRAY_SIZE(meshes);
    int i;

    assert(ARRAY_SIZE(materials) == num_meshes);

    root = mat4_scale(0.5f, 0.5f, 0.5f);
    for (i = 0; i < num_meshes; i++) {
        model = metalness_create_model(meshes[i], root, materials[i], env_name);
        darray_push(models, model);
    }

    scene = (scene_t*)malloc(sizeof(scene_t));
    scene->background = background;
    scene->skybox     = NULL;
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
            {1, 1, 1, 1},
            1,
            1,
            0,
            "assets/ponycar/body_basecolor.tga",
            "assets/ponycar/body_metallic.tga",
            "assets/ponycar/body_roughness.tga",
            "assets/ponycar/body_normal.tga",
            NULL,
            "assets/ponycar/body_emissive.tga",
            0,
            0,
        },
        {
            {1, 1, 1, 1},
            1,
            1,
            0,
            "assets/ponycar/interior_basecolor.tga",
            "assets/ponycar/interior_metallic.tga",
            "assets/ponycar/interior_roughness.tga",
            "assets/ponycar/interior_normal.tga",
            NULL,
            NULL,
            0,
            0,
        },
        {
            {1, 1, 1, 0.5f},
            1,
            1,
            0,
            "assets/ponycar/body_basecolor.tga",
            "assets/ponycar/body_metallic.tga",
            "assets/ponycar/body_roughness.tga",
            "assets/ponycar/body_normal.tga",
            NULL,
            "assets/ponycar/body_emissive.tga",
            0,
            1,
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
    scene->skybox     = NULL;
    scene->models     = models;

    return scene;
}
