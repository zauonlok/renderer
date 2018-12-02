#include "constant_shader.h"
#include <stdlib.h>
#include "../core/apis.h"

/* low-level apis */

vec4_t constant_vertex_shader(void *attribs_, void *varyings_, void *uniforms_) {
    constant_attribs_t *attribs = (constant_attribs_t*)attribs_;
    constant_varyings_t *varyings = (constant_varyings_t*)varyings_;
    constant_uniforms_t *uniforms = (constant_uniforms_t*)uniforms_;

    vec4_t local_pos = vec4_from_vec3(attribs->position, 1);
    vec4_t clip_pos = mat4_mul_vec4(uniforms->mvp_matrix, local_pos);
    varyings->texcoord = attribs->texcoord;
    return clip_pos;
}

static vec4_t calculate_emission(vec2_t uv, constant_uniforms_t *uniforms) {
    if (uniforms->emission_texture) {
        vec4_t emission_fac = uniforms->emission_factor;
        vec4_t emission_tex = texture_sample(uniforms->emission_texture, uv);
        return vec4_modulate(emission_fac, emission_tex);
    } else {
        return uniforms->emission_factor;
    }
}

vec4_t constant_fragment_shader(void *varyings_, void *uniforms_) {
    constant_varyings_t *varyings = (constant_varyings_t*)varyings_;
    constant_uniforms_t *uniforms = (constant_uniforms_t*)uniforms_;

    vec4_t ambient = uniforms->ambient_factor;
    vec4_t emission = calculate_emission(varyings->texcoord, uniforms);

    float color_r = ambient.x + emission.x;
    float color_g = ambient.y + emission.y;
    float color_b = ambient.z + emission.z;

    return vec4_new(color_r, color_g, color_b, 1);
}

/* high-level apis */

model_t *constant_create_model(mat4_t transform, const char *mesh,
                               constant_material_t material) {
    int sizeof_attribs = sizeof(constant_attribs_t);
    int sizeof_varyings = sizeof(constant_varyings_t);
    int sizeof_uniforms = sizeof(constant_uniforms_t);
    constant_uniforms_t *uniforms;
    program_t *program;
    model_t *model;

    program = program_create(constant_vertex_shader, constant_fragment_shader,
                             sizeof_attribs, sizeof_varyings, sizeof_uniforms);
    uniforms = (constant_uniforms_t*)program->uniforms;
    uniforms->ambient_factor = material.ambient_factor;
    uniforms->emission_factor = material.emission_factor;
    if (material.emission_texture) {
        const char *emission_filename = material.emission_texture;
        uniforms->emission_texture = texture_from_file(emission_filename);
    }

    model = (model_t*)malloc(sizeof(model_t));
    model->transform = transform;
    model->mesh      = mesh_load(mesh);
    model->program   = program;

    return model;
}

void constant_release_model(model_t *model) {
    constant_uniforms_t *uniforms = constant_get_uniforms(model);
    if (uniforms->emission_texture) {
        texture_release(uniforms->emission_texture);
    }
    program_release(model->program);
    mesh_release(model->mesh);
    free(model);
}

constant_uniforms_t *constant_get_uniforms(model_t *model) {
    return (constant_uniforms_t*)model->program->uniforms;
}

void constant_draw_model(framebuffer_t *framebuffer, model_t *model) {
    program_t *program = model->program;
    mesh_t *mesh = model->mesh;
    int num_faces = mesh_get_num_faces(mesh);
    constant_attribs_t *attribs;
    int i, j;

    for (i = 0; i < num_faces; i++) {
        for (j = 0; j < 3; j++) {
            attribs = (constant_attribs_t*)program->attribs[j];
            attribs->position = mesh_get_position(mesh, i, j);
            attribs->texcoord = mesh_get_texcoord(mesh, i, j);
        }
        graphics_draw_triangle(framebuffer, program);
    }
}
