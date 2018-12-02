#include "phong_models.h"
#include <stddef.h>
#include "../core/apis.h"
#include "../shaders/phong_shader.h"

model_t **phong_demon_hunter_models(void) {
    const char *body_obj = "assets/demon_hunter/body.obj";
    const char *crossbow_obj = "assets/demon_hunter/crossbow.obj";
    const char *eye_obj = "assets/demon_hunter/eye.obj";
    const char *floor_obj = "assets/demon_hunter/floor.obj";
    const char *body_tga = "assets/demon_hunter/body.tga";
    const char *crossbow_tga = "assets/demon_hunter/crossbow.tga";
    mat4_t body_transform = {{
        {1.000000f, 0.000000f, 0.000000f, 0.000000f},
        {0.000000f, 1.000000f, -0.000000f, 0.000000f},
        {0.000000f, 0.000000f, 1.000000f, 0.000000f},
        {0.000000f, 0.000000f, 0.000000f, 1.000000f},
    }};
    mat4_t crossbow0_transform = {{
        {0.304000f, -0.651348f, -0.695220f, 5.583765f},
        {0.299317f, 0.758106f, -0.579382f, 0.554488f},
        {0.904430f, -0.031959f, 0.425424f, -0.806390f},
        {0.000000f, 0.000000f, 0.000000f, 1.000000f},
    }};
    mat4_t crossbow1_transform = {{
        {0.253946f, 0.731762f, 0.632483f, -6.659619f},
        {-0.102114f, 0.670549f, -0.734804f, 2.202711f},
        {-0.961813f, 0.122015f, 0.245007f, -2.838731f},
        {0.000000f, 0.000000f, 0.000000f, 1.000000f},
    }};
    mat4_t eye0_transform = {{
        {1.000000f, 0.000000f, 0.000000f, -0.178922f},
        {0.000000f, 0.000000f, 1.000000f, 13.727595f},
        {0.000000f, -1.000000f, 0.000000f, 0.756335f},
        {0.000000f, 0.000000f, 0.000000f, 1.000000f},
    }};
    mat4_t eye1_transform = {{
        {1.000000f, 0.000000f, 0.000000f, -0.658013f},
        {0.000000f, 0.000000f, 1.000000f, 13.744929f},
        {0.000000f, -1.000000f, 0.000000f, 0.585421f},
        {0.000000f, 0.000000f, 0.000000f, 1.000000f},
    }};
    mat4_t floor_transform = {{
        {0.419282f, 0.000000f, 0.000000f, 0.004677f},
        {0.000000f, 0.000000f, 1.000000f, -0.203365f},
        {0.000000f, -0.419282f, 0.000000f, 0.099573f},
        {0.000000f, 0.000000f, 0.000000f, 1.000000f},
    }};
    phong_material_t material;
    model_t **models = NULL;
    model_t *model;
    mat4_t scale, translation, root;
    mat4_t transform;

    translation = mat4_translate(0.260f, -7.300f, -0.100f);
    scale = mat4_scale(0.127f, 0.127f, 0.127f);
    root = mat4_mul_mat4(scale, translation);

    material.ambient_factor = vec4_new(0.1f, 0.1f, 0.1f, 1);
    material.diffuse_factor = vec4_new(1, 1, 1, 1);
    material.emission_factor = vec4_new(0, 0, 0, 1);
    material.specular_factor = vec4_new(0.15f, 0.15f, 0.15f, 1);
    material.shininess = 16;
    material.diffuse_texture = NULL;
    material.emission_texture = NULL;
    material.specular_texture = NULL;

    material.diffuse_texture = body_tga;
    transform = mat4_mul_mat4(root, body_transform);
    model = phong_create_model(transform, body_obj, material);
    darray_push(models, model);

    material.diffuse_texture = crossbow_tga;
    transform = mat4_mul_mat4(root, crossbow0_transform);
    model = phong_create_model(transform, crossbow_obj, material);
    darray_push(models, model);
    transform = mat4_mul_mat4(root, crossbow1_transform);
    model = phong_create_model(transform, crossbow_obj, material);
    darray_push(models, model);

    material.diffuse_factor = vec4_new(0, 0, 0, 1);
    material.emission_factor = vec4_new(1, 1, 1, 1);
    material.diffuse_texture = NULL;
    transform = mat4_mul_mat4(root, eye0_transform);
    model = phong_create_model(transform, eye_obj, material);
    darray_push(models, model);
    transform = mat4_mul_mat4(root, eye1_transform);
    model = phong_create_model(transform, eye_obj, material);
    darray_push(models, model);

    material.diffuse_factor = vec4_new(0.4f, 0.4f, 0.4f, 1);
    material.emission_factor = vec4_new(0, 0, 0, 1);
    transform = mat4_mul_mat4(root, floor_transform);
    model = phong_create_model(transform, floor_obj, material);
    darray_push(models, model);

    return models;
}

void phong_release_models(model_t **models) {
    int num_models = darray_size(models);
    int i;
    for (i = 0; i < num_models; i++) {
        phong_release_model(models[i]);
    }
    darray_free(models);
}
