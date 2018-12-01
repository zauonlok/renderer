#include "blinn_models.h"
#include <stddef.h>
#include "../core/apis.h"
#include "../shaders/blinn_shader.h"

model_t **blinn_craftsman_models(void) {
    const char *anvil_obj = "assets/craftsman/anvil.obj";
    const char *floor_obj = "assets/craftsman/floor.obj";
    const char *hammer_obj = "assets/craftsman/hammer.obj";
    const char *hotiron_obj = "assets/craftsman/hotiron.obj";
    const char *shoulderpad0_obj = "assets/craftsman/shoulderpad0.obj";
    const char *shoulderpad1_obj = "assets/craftsman/shoulderpad1.obj";
    const char *smith_obj = "assets/craftsman/smith.obj";
    const char *anvil_basecolor_tga = "assets/craftsman/anvil_basecolor.tga";
    const char *floor_basecolor_tga = "assets/craftsman/floor_basecolor.tga";
    const char *smith_basecolor_tga = "assets/craftsman/smith_basecolor.tga";
    const char *smith_emissive_tga = "assets/craftsman/smith_emissive.tga";
    mat4_t anvil_transform = {{
       {0.936571f, 0.000000f, 0.000000f, 0.000000f},
       {0.000000f, 0.000000f, 0.936571f, -0.000000f},
       {0.000000f, -0.936571f, 0.000000f, 23.366537f},
       {0.000000f, 0.000000f, 0.000000f, 1.000000f},
    }};
    mat4_t floor_transform = {{
       {1.259828f, 0.000000f, 0.000000f, 1.668093f},
       {0.000000f, 0.000000f, 0.885767f, -0.000000f},
       {0.000000f, -1.259828f, 0.000000f, 10.833580f},
       {0.000000f, 0.000000f, 0.000000f, 1.000000f},
    }};
    mat4_t hammer_transform = {{
       {0.324427f, 0.217420f, 0.920584f, 11.052060f},
       {-0.852411f, 0.489096f, 0.184888f, 78.562383f},
       {-0.410056f, -0.844697f, 0.344006f, -8.390521f},
       {0.000000f, 0.000000f, 0.000000f, 1.000000f},
    }};
    mat4_t hotiron_transform = {{
       {0.840193f, -0.540788f, -0.040288f, -34.668739f},
       {-0.102606f, -0.085585f, -0.991035f, 17.130267f},
       {0.532492f, 0.836793f, -0.127395f, 56.477256f},
       {0.000000f, 0.000000f, 0.000000f, 1.000000f},
    }};
    mat4_t shoulderpad0_transform = {{
       {1.000000f, 0.000000f, 0.000000f, 0.000000f},
       {0.000000f, 0.000000f, 1.000000f, 0.000000f},
       {0.000000f, -1.000000f, 0.000000f, -0.000000f},
       {0.000000f, 0.000000f, 0.000000f, 1.000000f},
    }};
    mat4_t shoulderpad1_transform = {{
       {0.954568f, -0.168932f, -0.248769f, 10.987287f},
       {0.171253f, -0.392511f, 0.908502f, 8.632905f},
       {-0.243870f, -0.936879f, -0.335766f, 10.008259f},
       {0.000000f, 0.000000f, 0.000000f, 1.000000f},
    }};
    mat4_t smith_transform = {{
       {1.000000f, 0.000000f, 0.000000f, 0.000000f},
       {0.000000f, 0.000000f, 1.000000f, 0.000000f},
       {0.000000f, -1.000000f, 0.000000f, 0.000000f},
       {0.000000f, 0.000000f, 0.000000f, 1.000000f},
    }};
    blinn_material_t material;
    model_t **models = NULL;
    model_t *model;
    mat4_t scale, translation, root;
    mat4_t transform;

    scale = mat4_scale(0.037f, 0.037f, 0.037f);
    translation = mat4_translate(0, -1, 0);
    root = mat4_mul_mat4(translation, scale);

    material.ambient_factor = vec4_new(0.1f, 0.1f, 0.1f, 1);
    material.diffuse_factor = vec4_new(1, 1, 1, 1);
    material.emission_factor = vec4_new(0, 0, 0, 1);
    material.specular_factor = vec4_new(0.15f, 0.15f, 0.15f, 1);
    material.shininess = 16;
    material.diffuse_texture = NULL;
    material.emission_texture = NULL;
    material.specular_texture = NULL;

    material.diffuse_texture = anvil_basecolor_tga;
    transform = mat4_mul_mat4(root, anvil_transform);
    model = blinn_create_model(transform, anvil_obj, material);
    darray_push(models, model);

    material.diffuse_texture = floor_basecolor_tga;
    transform = mat4_mul_mat4(root, floor_transform);
    model = blinn_create_model(transform, floor_obj, material);
    darray_push(models, model);

    material.emission_factor = vec4_new(1, 1, 1, 1);
    material.diffuse_texture = smith_basecolor_tga;
    material.emission_texture = smith_emissive_tga;

    transform = mat4_mul_mat4(root, hammer_transform);
    model = blinn_create_model(transform, hammer_obj, material);
    darray_push(models, model);

    transform = mat4_mul_mat4(root, hotiron_transform);
    model = blinn_create_model(transform, hotiron_obj, material);
    darray_push(models, model);

    transform = mat4_mul_mat4(root, shoulderpad0_transform);
    model = blinn_create_model(transform, shoulderpad0_obj, material);
    darray_push(models, model);

    transform = mat4_mul_mat4(root, shoulderpad1_transform);
    model = blinn_create_model(transform, shoulderpad1_obj, material);
    darray_push(models, model);

    transform = mat4_mul_mat4(root, smith_transform);
    model = blinn_create_model(transform, smith_obj, material);
    darray_push(models, model);

    return models;
}

void blinn_release_models(model_t **models) {
    int num_models = darray_size(models);
    int i;
    for (i = 0; i < num_models; i++) {
        blinn_release_model(models[i]);
    }
    darray_free(models);
}
