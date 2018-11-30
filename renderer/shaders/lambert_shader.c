#include "lambert_shader.h"
#include <stdlib.h>
#include "../core/apis.h"

/* low-level apis */

vec4_t lambert_vertex_shader(void *attribs_, void *varyings_, void *uniforms_) {
    lambert_attribs_t *attribs = (lambert_attribs_t*)attribs_;
    lambert_varyings_t *varyings = (lambert_varyings_t*)varyings_;
    lambert_uniforms_t *uniforms = (lambert_uniforms_t*)uniforms_;

    vec4_t local_pos = vec4_from_vec3(attribs->local_pos, 1);
    vec4_t world_pos = mat4_mul_vec4(uniforms->model_matrix, local_pos);
    vec4_t clip_pos = mat4_mul_vec4(uniforms->viewproj_matrix, world_pos);

    vec4_t local_normal = vec4_from_vec3(attribs->normal, 0);
    mat4_t normal_matrix = uniforms->model_it_matrix;
    vec4_t world_normal = mat4_mul_vec4(normal_matrix, local_normal);

    varyings->world_pos = vec3_from_vec4(world_pos);
    varyings->texcoord = attribs->texcoord;
    varyings->normal = vec3_from_vec4(world_normal);
    return clip_pos;
}

static float max_float(float a, float b) {
    return a > b ? a : b;
}

static vec4_t calculate_diffuse(vec2_t uv, lambert_uniforms_t *uniforms) {
    if (uniforms->diffuse_texture) {
        vec4_t diffuse_fac = uniforms->diffuse_factor;
        vec4_t diffuse_tex = texture_sample(uniforms->diffuse_texture, uv);
        return vec4_modulate(diffuse_fac, diffuse_tex);
    } else {
        return uniforms->diffuse_factor;
    }
}

static vec4_t calculate_emission(vec2_t uv, lambert_uniforms_t *uniforms) {
    if (uniforms->emission_texture) {
        vec4_t emission_fac = uniforms->emission_factor;
        vec4_t emission_tex = texture_sample(uniforms->emission_texture, uv);
        return vec4_modulate(emission_fac, emission_tex);
    } else {
        return uniforms->emission_factor;
    }
}

vec4_t lambert_fragment_shader(void *varyings_, void *uniforms_) {
    lambert_varyings_t *varyings = (lambert_varyings_t*)varyings_;
    lambert_uniforms_t *uniforms = (lambert_uniforms_t*)uniforms_;

    vec4_t ambient = uniforms->ambient_factor;
    vec4_t diffuse = calculate_diffuse(varyings->texcoord, uniforms);
    vec4_t emission = calculate_emission(varyings->texcoord, uniforms);

    vec3_t normal = vec3_normalize(varyings->normal);
    vec3_t light_dir = vec3_normalize(uniforms->light_dir);
    float cos_factor = max_float(-vec3_dot(light_dir, normal), 0);

    float color_r = ambient.x + diffuse.x * cos_factor + emission.x;
    float color_g = ambient.y + diffuse.y * cos_factor + emission.y;
    float color_b = ambient.z + diffuse.z * cos_factor + emission.z;

    return vec4_new(color_r, color_g, color_b, 1);
}

/* high-level apis */

model_t *lambert_create_model(mat4_t transform, const char *mesh,
                              lambert_material_t material) {
    int sizeof_attribs = sizeof(lambert_attribs_t);
    int sizeof_varyings = sizeof(lambert_varyings_t);
    int sizeof_uniforms = sizeof(lambert_uniforms_t);
    lambert_uniforms_t *uniforms;
    program_t *program;
    model_t *model;

    program = program_create(lambert_vertex_shader, lambert_fragment_shader,
                             sizeof_attribs, sizeof_varyings, sizeof_uniforms);
    uniforms = (lambert_uniforms_t*)program->uniforms;
    uniforms->ambient_factor = material.ambient_factor;
    uniforms->diffuse_factor = material.diffuse_factor;
    uniforms->emission_factor = material.emission_factor;
    if (material.diffuse_texture) {
        const char *diffuse_filename = material.diffuse_texture;
        uniforms->diffuse_texture = texture_from_file(diffuse_filename);
    }
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

void lambert_release_model(model_t *model) {
    lambert_uniforms_t *uniforms = lambert_get_uniforms(model);
    if (uniforms->diffuse_texture) {
        texture_release(uniforms->diffuse_texture);
    }
    if (uniforms->emission_texture) {
        texture_release(uniforms->emission_texture);
    }
    program_release(model->program);
    mesh_release(model->mesh);
    free(model);
}

lambert_uniforms_t *lambert_get_uniforms(model_t *model) {
    return (lambert_uniforms_t*)model->program->uniforms;
}

void lambert_draw_model(framebuffer_t *framebuffer, model_t *model) {
    program_t *program = model->program;
    mesh_t *mesh = model->mesh;
    int num_faces = mesh_get_num_faces(mesh);
    lambert_attribs_t *attribs;
    int i, j;

    for (i = 0; i < num_faces; i++) {
        for (j = 0; j < 3; j++) {
            attribs = (lambert_attribs_t*)program->attribs[j];
            attribs->local_pos = mesh_get_position(mesh, i, j);
            attribs->texcoord = mesh_get_texcoord(mesh, i, j);
            attribs->normal = mesh_get_normal(mesh, i, j);
        }
        graphics_draw_triangle(framebuffer, program);
    }
}
