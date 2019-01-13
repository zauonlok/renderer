#include "lambert_models.h"
#include <stddef.h>
#include "../core/apis.h"
#include "../shaders/lambert_shader.h"

model_t **lambert_elf_girl_models(void) {
    const char *face0_obj = "assets/elf_girl/face0.obj";
    const char *face1_obj = "assets/elf_girl/face1.obj";
    const char *body0_obj = "assets/elf_girl/body0.obj";
    const char *body1_obj = "assets/elf_girl/body1.obj";
    const char *body2_obj = "assets/elf_girl/body2.obj";
    const char *hair_obj = "assets/elf_girl/hair.obj";
    const char *ce_obj = "assets/elf_girl/ce.obj";
    const char *face_tga = "assets/elf_girl/face.tga";
    const char *body_tga = "assets/elf_girl/body.tga";
    const char *hair_tga = "assets/elf_girl/hair.tga";
    const char *ce_tga = "assets/elf_girl/ce.tga";
    lambert_material_t material;
    model_t **models = NULL;
    model_t *model;
    mat4_t scale, rotation, translation;
    mat4_t transform;

    translation = mat4_translate(2.449f, -2.472f, -20.907f);
    scale = mat4_scale(0.046f, 0.046f, 0.046f);
    rotation = mat4_rotate_x(TO_RADIANS(-90));
    transform = mat4_mul_mat4(rotation, mat4_mul_mat4(scale, translation));

    material.ambient_factor = vec4_new(0.1f, 0.1f, 0.1f, 1);
    material.diffuse_factor = vec4_new(1, 1, 1, 1);
    material.emission_factor = vec4_new(0, 0, 0, 1);
    material.diffuse_texture = NULL;
    material.emission_texture = NULL;

    material.diffuse_texture = face_tga;
    model = lambert_create_model(transform, face0_obj, material);
    darray_push(models, model);
    model = lambert_create_model(transform, face1_obj, material);
    darray_push(models, model);

    material.diffuse_texture = body_tga;
    model = lambert_create_model(transform, body0_obj, material);
    darray_push(models, model);
    model = lambert_create_model(transform, body1_obj, material);
    darray_push(models, model);
    model = lambert_create_model(transform, body2_obj, material);
    darray_push(models, model);

    material.diffuse_texture = hair_tga;
    model = lambert_create_model(transform, hair_obj, material);
    darray_push(models, model);

    material.diffuse_texture = ce_tga;
    model = lambert_create_model(transform, ce_obj, material);
    darray_push(models, model);

    return models;
}

void lambert_release_models(model_t **models) {
    int num_models = darray_size(models);
    int i;
    for (i = 0; i < num_models; i++) {
        lambert_release_model(models[i]);
    }
    darray_free(models);
}
