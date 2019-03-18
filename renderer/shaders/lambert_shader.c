#include <stdlib.h>
#include "../core/api.h"
#include "lambert_shader.h"
#include "shader_helper.h"

/* low-level api */

vec4_t lambert_vertex_shader(void *attribs_, void *varyings_, void *uniforms_) {
    lambert_attribs_t *attribs = (lambert_attribs_t*)attribs_;
    lambert_varyings_t *varyings = (lambert_varyings_t*)varyings_;
    lambert_uniforms_t *uniforms = (lambert_uniforms_t*)uniforms_;

    vec4_t local_pos = vec4_from_vec3(attribs->position, 1);
    vec4_t clip_pos = mat4_mul_vec4(uniforms->mvp_matrix, local_pos);

    vec3_t local_normal = attribs->normal;
    vec3_t world_normal = mat3_mul_vec3(uniforms->normal_matrix, local_normal);

    varyings->texcoord = attribs->texcoord;
    varyings->normal = world_normal;
    return clip_pos;
}

vec4_t lambert_fragment_shader(void *varyings_, void *uniforms_, int *discard) {
    lambert_varyings_t *varyings = (lambert_varyings_t*)varyings_;
    lambert_uniforms_t *uniforms = (lambert_uniforms_t*)uniforms_;

    vec3_t color = vec3_new(0, 0, 0);
    float alpha = 1;

    if (uniforms->diffuse) {
        vec4_t sample = texture_sample(uniforms->diffuse, varyings->texcoord);
        vec3_t albedo = vec3_from_vec4(sample);

        vec3_t normal = vec3_normalize(varyings->normal);
        vec3_t light_dir = vec3_negate(uniforms->light_dir);
        float strength = float_max(vec3_dot(normal, light_dir), 0);
        vec3_t diffuse = vec3_mul(albedo, strength + uniforms->ambient);

        color = vec3_add(color, diffuse);
        alpha = sample.w;
    }

    if (uniforms->alpha_cutoff > 0 && alpha < uniforms->alpha_cutoff) {
        *discard = 1;
        return vec4_new(0, 0, 0, 0);
    }

    if (uniforms->emission) {
        vec4_t sample = texture_sample(uniforms->emission, varyings->texcoord);
        vec3_t emission = vec3_from_vec4(sample);
        color = vec3_add(color, emission);
    }

    return vec4_from_vec3(color, alpha);
}

/* high-level api */

static void draw_model(model_t *model, framebuffer_t *framebuffer) {
    program_t *program = model->program;
    mesh_t *mesh = model->mesh;
    int num_faces = mesh_get_num_faces(mesh);
    lambert_attribs_t *attribs;
    int i, j;

    for (i = 0; i < num_faces; i++) {
        for (j = 0; j < 3; j++) {
            vertex_t vertex = mesh_get_vertex(mesh, i, j);
            attribs = (lambert_attribs_t*)program_get_attribs(program, j);
            attribs->position = vertex.position;
            attribs->texcoord = vertex.texcoord;
            attribs->normal = vertex.normal;
        }
        graphics_draw_triangle(framebuffer, program);
    }
}

static void release_model(model_t *model) {
    lambert_uniforms_t *uniforms;
    uniforms = (lambert_uniforms_t*)program_get_uniforms(model->program);
    cache_release_texture(uniforms->emission);
    cache_release_texture(uniforms->diffuse);
    program_release(model->program);
    cache_release_mesh(model->mesh);
    free(model);
}

model_t *lambert_create_model(const char *mesh, mat4_t transform,
                              lambert_material_t material) {
    int sizeof_attribs = sizeof(lambert_attribs_t);
    int sizeof_varyings = sizeof(lambert_varyings_t);
    int sizeof_uniforms = sizeof(lambert_uniforms_t);
    lambert_uniforms_t *uniforms;
    program_t *program;
    model_t *model;

    program = program_create(lambert_vertex_shader, lambert_fragment_shader,
                             sizeof_attribs, sizeof_varyings, sizeof_uniforms,
                             material.double_sided, material.enable_blend);

    uniforms = (lambert_uniforms_t*)program_get_uniforms(program);
    uniforms->ambient = material.ambient;
    uniforms->alpha_cutoff = material.alpha_cutoff;
    uniforms->emission = cache_acquire_texture(material.emission, 0);
    uniforms->diffuse = cache_acquire_texture(material.diffuse, 0);

    model = (model_t*)malloc(sizeof(model_t));
    model->mesh      = cache_acquire_mesh(mesh);
    model->transform = transform;
    model->program   = program;
    model->draw      = draw_model;
    model->release   = release_model;
    model->opaque    = !material.enable_blend;

    return model;
}

void lambert_update_uniforms(model_t *model, vec3_t light_dir,
                             mat4_t mvp_matrix, mat3_t normal_matrix) {
    lambert_uniforms_t *uniforms;
    uniforms = (lambert_uniforms_t*)program_get_uniforms(model->program);
    uniforms->light_dir = light_dir;
    uniforms->mvp_matrix = mvp_matrix;
    uniforms->normal_matrix = normal_matrix;
}
