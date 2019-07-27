#include <assert.h>
#include <stdio.h>
#include "../core/api.h"
#include "../shaders/pbr_shader.h"
#include "../shaders/skybox_shader.h"
#include "pbr_scenes.h"
#include "scene_helper.h"

scene_t *pbr_assassin_scene(void) {
    mat4_t translation = mat4_translate(0, -125.815f, 18.898f);
    mat4_t scale = mat4_scale(0.0038f, 0.0038f, 0.0038f);
    mat4_t root = mat4_mul_mat4(scale, translation);
    return helper_load_pbrm_scene("assassin/assassin.scn", root);
}

scene_t *pbr_buster_scene(void) {
    const char *skeleton = "buster/buster.ani";
    int mesh2node[39] = {
        2, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
        18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
        31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43,
    };
    int mesh2material[39] = {
        0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2,
        1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    };
    pbrm_material_t materials[] = {
        {
            {1, 1, 1, 1}, 0, 1,
            "buster/boden_basecolor.tga",
            NULL,
            "buster/boden_roughness.tga",
            NULL,
            NULL,
            NULL,
            1, 1, 0,
        },
        {
            {1, 1, 1, 1}, 1, 1,
            "buster/body_basecolor.tga",
            "buster/body_metalness.tga",
            "buster/body_roughness.tga",
            NULL,
            "buster/body_occlusion.tga",
            "buster/body_emission.tga",
            0, 0, 1,
        },
        {
            {1, 1, 1, 1}, 1, 1,
            "buster/legs_basecolor.tga",
            "buster/legs_metalness.tga",
            "buster/legs_roughness.tga",
            NULL,
            "buster/legs_occlusion.tga",
            NULL,
            0, 0, 0,
        },
    };
    vec3_t background = vec3_new(0.196f, 0.196f, 0.196f);
    const char *env_name = "papermill";
    mat4_t scale, rotation, translation, root;
    int num_meshes = ARRAY_SIZE(mesh2node);
    model_t **models = NULL;
    model_t *model;
    int i;

    assert(ARRAY_SIZE(mesh2material) == num_meshes);

    translation = mat4_translate(0, 15.918f, -5.720f);
    rotation = mat4_rotate_x(TO_RADIANS(90));
    scale = mat4_scale(0.0045f, 0.0045f, 0.0045f);
    root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    for (i = 1; i < num_meshes; i++) {
        int node_index = mesh2node[i];
        int material_index = mesh2material[i];
        pbrm_material_t material = materials[material_index];
        char obj_filepath[64];
        sprintf(obj_filepath, "buster/buster%d.obj", i);
        model = pbrm_create_model(obj_filepath, skeleton, node_index, root,
                                  material, env_name);
        darray_push(models, model);
    }

    return scene_create(background, NULL, models, 1, 1, 0);
}

scene_t *pbr_crab_scene(void) {
    mat4_t translation = mat4_translate(-0.285f, 0.780f, 0.572f);
    mat4_t rotation = mat4_rotate_y(TO_RADIANS(180));
    mat4_t scale = mat4_scale(0.167f, 0.167f, 0.167f);
    mat4_t root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    return helper_load_pbrs_scene("crab/crab.scn", root);
}

scene_t *pbr_dieselpunk_scene(void) {
    mat4_t translation = mat4_translate(1.036f, -114.817f, 27.682f);
    mat4_t rotation = mat4_rotate_y(TO_RADIANS(-90));
    mat4_t scale = mat4_scale(0.0012f, 0.0012f, 0.0012f);
    mat4_t root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    return helper_load_pbrm_scene("dieselpunk/dieselpunk.scn", root);
}

scene_t *pbr_drone_scene(void) {
    const char *drone_mesh = "drone/drone.obj";
    const char *drone_skeleton = "drone/drone.ani";
    pbrs_material_t drone_material = {
        {1, 1, 1, 1}, {1, 1, 1}, 1,
        "drone/drone_diffuse.tga",
        "drone/drone_specular.tga",
        "drone/drone_glossiness.tga",
        NULL,
        "drone/drone_occlusion.tga",
        "drone/drone_emission.tga",
        0, 0, 0,
    };
    const char *fire_skeleton = "drone/fire.ani";
    int fire_mesh2node[15] = {
        5, 7, 9, 11, 13, 16, 18, 20, 22, 24, 27, 29, 31, 33, 35,
    };
    pbrs_material_t fire_material = {
        {0, 0, 0, 0.1f}, {0, 0, 0}, 0.8f,
        "drone/fire_diffuse.tga",
        NULL,
        NULL,
        NULL,
        NULL,
        "drone/fire_emission.tga",
        1, 1, 0,
    };
    vec3_t background = vec3_new(0.196f, 0.196f, 0.196f);
    const char *env_name = "papermill";
    mat4_t scale, rotation, translation, root;
    int num_fires = ARRAY_SIZE(fire_mesh2node);
    model_t **models = NULL;
    model_t *model;
    int i;

    translation = mat4_translate(0, -78.288f, -4.447f);
    rotation = mat4_rotate_y(TO_RADIANS(180));
    scale = mat4_scale(0.0028f, 0.0028f, 0.0028f);
    root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    model = pbrs_create_model(drone_mesh, drone_skeleton, -1, root,
                              drone_material, env_name);
    darray_push(models, model);

    root = mat4_mul_mat4(root, mat4_rotate_x(TO_RADIANS(90)));
    for (i = 0; i < num_fires; i++) {
        int node_index = fire_mesh2node[i];
        char obj_filepath[64];
        sprintf(obj_filepath, "drone/fire%d.obj", i);
        model = pbrs_create_model(obj_filepath, fire_skeleton, node_index, root,
                                  fire_material, env_name);
        darray_push(models, model);
    }

    return scene_create(background, NULL, models, 1, 1, 0);
}

scene_t *pbr_helmet_scene(void) {
    mat4_t translation = mat4_translate(0.002f, 0.187f, 0);
    mat4_t rotation = mat4_rotate_x(TO_RADIANS(90));
    mat4_t scale = mat4_scale(0.5f, 0.5f, 0.5f);
    mat4_t root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    return helper_load_pbrm_scene("helmet/helmet.scn", root);
}

scene_t *pbr_junkrat_scene(void) {
    const char *skeleton = "junkrat/junkrat.ani";
    pbrm_material_t materials[] = {
        {
            {1, 1, 1, 1}, 1, 0.6f,
            "junkrat/upper_basecolor.tga",
            "junkrat/upper_metalness.tga",
            NULL,
            NULL,
            NULL,
            NULL,
            0, 0, 0,
        },
        {
            {1, 1, 1, 1}, 1, 0.6f,
            "junkrat/lower_basecolor.tga",
            "junkrat/lower_metalness.tga",
            NULL,
            NULL,
            NULL,
            NULL,
            0, 0, 0,
        },
        {
            {1, 1, 1, 1}, 0, 1,
            "junkrat/head_basecolor.tga",
            NULL,
            "junkrat/head_roughness.tga",
            NULL,
            NULL,
            NULL,
            0, 0, 0,
        },
        {
            {1, 1, 1, 1}, 1, 0.6f,
            "junkrat/back_basecolor.tga",
            "junkrat/back_metalness.tga",
            NULL,
            NULL,
            NULL,
            NULL,
            0, 0, 0,
        },
    };
    int mesh2material[63] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 2, 0, 1, 0, 2, 1,
        0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2,
    };
    vec3_t background = vec3_new(0.196f, 0.196f, 0.196f);
    const char *env_name = "papermill";
    mat4_t scale, translation, root;
    int num_meshes = ARRAY_SIZE(mesh2material);
    model_t **models = NULL;
    model_t *model;
    int i;

    translation = mat4_translate(3.735f, -382.993f, 57.980f);
    scale = mat4_scale(0.0013f, 0.0013f, 0.0013f);
    root = mat4_mul_mat4(scale, translation);
    for (i = 0; i < num_meshes; i++) {
        int material_index = mesh2material[i];
        pbrm_material_t material = materials[material_index];
        char obj_filepath[64];
        sprintf(obj_filepath, "junkrat/junkrat%d.obj", i);
        model = pbrm_create_model(obj_filepath, skeleton, -1, root,
                                  material, env_name);
        darray_push(models, model);
    }

    return scene_create(background, NULL, models, 1, 1, 0);
}

scene_t *pbr_ornitier_scene(void) {
    const char *meshes[] = {
        "ornitier/base.obj",
        "ornitier/body.obj",
        "ornitier/coat.obj",
        "ornitier/feuga.obj",
        "ornitier/hands.obj",
        "ornitier/hat.obj",
        "ornitier/legs.obj",
    };
    pbrm_material_t materials[] = {
        {
            {1, 1, 1, 1}, 1, 1,
            "ornitier/base_basecolor.tga",
            "ornitier/base_metalness.tga",
            "ornitier/base_roughness.tga",
            "ornitier/base_normal.tga",
            NULL,
            "ornitier/base_emission.tga",
            0, 0, 0,
        },
        {
            {1, 1, 1, 1}, 1, 1,
            "ornitier/body_basecolor.tga",
            NULL,
            NULL,
            NULL,
            NULL,
            "ornitier/body_emission.tga",
            0, 0, 0,
        },
        {
            {1, 1, 1, 1}, 1, 1,
            "ornitier/coat_basecolor.tga",
            "ornitier/coat_metalness.tga",
            "ornitier/coat_roughness.tga",
            NULL,
            NULL,
            NULL,
            0, 0, 0,
        },
        {
            {0.8f, 0.8f, 0.8f, 0.84f}, 0, 0.04f,
            "ornitier/feuga_basecolor.tga",
            NULL,
            NULL,
            NULL,
            NULL,
            "ornitier/feuga_emission.tga",
            0, 1, 0,
        },
        {
            {1, 1, 1, 1}, 1, 1,
            "ornitier/hands_basecolor.tga",
            "ornitier/hands_metalness.tga",
            "ornitier/hands_roughness.tga",
            "ornitier/hands_normal.tga",
            NULL,
            NULL,
            0, 0, 0,
        },
        {
            {1, 1, 1, 1}, 1, 1,
            "ornitier/hat_basecolor.tga",
            "ornitier/hat_metalness.tga",
            "ornitier/hat_roughness.tga",
            "ornitier/hat_normal.tga",
            NULL,
            NULL,
            0, 0, 0,
        },
        {
            {1, 1, 1, 1}, 1, 1,
            "ornitier/legs_basecolor.tga",
            "ornitier/legs_metalness.tga",
            "ornitier/legs_roughness.tga",
            "ornitier/legs_normal.tga",
            NULL,
            NULL,
            0, 0, 0,
        },
    };
    vec3_t background = vec3_new(0.196f, 0.196f, 0.196f);
    const char *env_name = "footprint";
    mat4_t scale, translation, root;
    int num_meshes = ARRAY_SIZE(meshes);
    model_t **models = NULL;
    model_t *model;
    int i;

    assert(ARRAY_SIZE(materials) == num_meshes);

    translation = mat4_translate(-111.550f, -67.795f, 178.647f);
    scale = mat4_scale(0.00095f, 0.00095f, 0.00095f);
    root = mat4_mul_mat4(scale, translation);
    for (i = 0; i < num_meshes; i++) {
        model = pbrm_create_model(meshes[i], NULL, -1, root,
                                  materials[i], env_name);
        darray_push(models, model);
    }

    return scene_create(background, NULL, models, 1, 1, 0);
}

scene_t *pbr_ponycar_scene(void) {
    mat4_t translation = mat4_translate(-10.343f, -13.252f, -186.343f);
    mat4_t rotation = mat4_rotate_x(TO_RADIANS(-90));
    mat4_t scale = mat4_scale(0.0015f, 0.0015f, 0.0015f);
    mat4_t root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    return helper_load_pbrm_scene("ponycar/ponycar.scn", root);
}

scene_t *pbr_sphere_scene(void) {
    const char *mesh = "common/sphere.obj";
    pbrm_material_t material = {
        {1, 1, 1, 1}, 1, 1, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0,
    };
    vec3_t background = vec3_new(0.196f, 0.196f, 0.196f);
    const char *env_name = "footprint";
    model_t **models = NULL;
    model_t *model;
    int i;

    for (i = 0; i <= 10; i++) {
        mat4_t translation = mat4_translate(1.5f * (i - 5), 1.5f, 0);
        mat4_t scale = mat4_scale(0.125f, 0.125f, 0.125f);
        mat4_t transform = mat4_mul_mat4(scale, translation);
        material.basecolor_factor = vec4_new(1, 1, 1, 1);
        material.metalness_factor = 1;
        material.roughness_factor = (float)i / 10;
        model = pbrm_create_model(mesh, NULL, -1, transform,
                                  material, env_name);
        darray_push(models, model);
    }

    for (i = 0; i <= 10; i++) {
        mat4_t translation = mat4_translate(1.5f * (i - 5), 0, 0);
        mat4_t scale = mat4_scale(0.125f, 0.125f, 0.125f);
        mat4_t transform = mat4_mul_mat4(scale, translation);
        material.basecolor_factor = vec4_new(1, 1, 1, 1);
        material.metalness_factor = 0;
        material.roughness_factor = (float)i / 10;
        model = pbrm_create_model(mesh, NULL, -1, transform,
                                  material, env_name);
        darray_push(models, model);
    }

    for (i = 0; i <= 10; i++) {
        mat4_t translation = mat4_translate(1.5f * (i - 5), -1.5f, 0);
        mat4_t scale = mat4_scale(0.125f, 0.125f, 0.125f);
        mat4_t transform = mat4_mul_mat4(scale, translation);
        material.basecolor_factor = vec4_new(0, 0, 0, 1);
        material.metalness_factor = 0;
        material.roughness_factor = (float)i / 10;
        model = pbrm_create_model(mesh, NULL, -1, transform,
                                  material, env_name);
        darray_push(models, model);
    }

    return scene_create(background, NULL, models, 1, 1, 0);
}
