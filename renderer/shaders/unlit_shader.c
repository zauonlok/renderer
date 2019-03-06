#include <stdlib.h>
#include "../core/api.h"
#include "cache_helper.h"
#include "unlit_shader.h"

/* low-level api */

vec4_t unlit_vertex_shader(void *attribs_, void *varyings_, void *uniforms_) {
    unlit_attribs_t *attribs = (unlit_attribs_t*)attribs_;
    unlit_varyings_t *varyings = (unlit_varyings_t*)varyings_;
    unlit_uniforms_t *uniforms = (unlit_uniforms_t*)uniforms_;

    vec4_t local_pos = vec4_from_vec3(attribs->position, 1);
    vec4_t clip_pos = mat4_mul_vec4(uniforms->mvp_matrix, local_pos);
    varyings->texcoord = attribs->texcoord;
    return clip_pos;
}

vec4_t unlit_fragment_shader(void *varyings_, void *uniforms_) {
    unlit_varyings_t *varyings = (unlit_varyings_t*)varyings_;
    unlit_uniforms_t *uniforms = (unlit_uniforms_t*)uniforms_;

    if (uniforms->texture) {
        vec4_t sample = texture_sample(uniforms->texture, varyings->texcoord);
        return vec4_modulate(uniforms->factor, sample);
    } else {
        return uniforms->factor;
    }
}

/* high-level api */

static unlit_uniforms_t *get_uniforms(model_t *model) {
    return (unlit_uniforms_t*)program_get_uniforms(model->program);
}

static void release_model(model_t *model) {
    unlit_uniforms_t *uniforms = get_uniforms(model);
    if (uniforms->texture) {
        cache_release_texture(uniforms->texture);
    }
    program_release(model->program);
    mesh_release(model->mesh);
    free(model);
}

static void draw_model(model_t *model, framebuffer_t *framebuffer) {
    program_t *program = model->program;
    mesh_t *mesh = model->mesh;
    int num_faces = mesh_get_num_faces(mesh);
    unlit_attribs_t *attribs;
    int i, j;

    for (i = 0; i < num_faces; i++) {
        for (j = 0; j < 3; j++) {
            vertex_t vertex = mesh_get_vertex(mesh, i, j);
            attribs = (unlit_attribs_t*)program_get_attribs(program, j);
            attribs->position = vertex.position;
            attribs->texcoord = vertex.texcoord;
        }
        graphics_draw_triangle(framebuffer, program);
    }
}

model_t *unlit_create_model(const char *mesh, mat4_t transform,
                            unlit_material_t material) {
    int sizeof_attribs = sizeof(unlit_attribs_t);
    int sizeof_varyings = sizeof(unlit_varyings_t);
    int sizeof_uniforms = sizeof(unlit_uniforms_t);
    unlit_uniforms_t *uniforms;
    program_t *program;
    model_t *model;

    program = program_create(unlit_vertex_shader, unlit_fragment_shader,
                             sizeof_attribs, sizeof_varyings, sizeof_uniforms,
                             material.double_sided, material.enable_blend);
    uniforms = (unlit_uniforms_t*)program_get_uniforms(program);
    uniforms->factor = material.factor;
    if (material.texture) {
        uniforms->texture = cache_acquire_texture(material.texture, 0);
    }

    model = (model_t*)malloc(sizeof(model_t));
    model->mesh      = mesh_load(mesh);
    model->transform = transform;
    model->program   = program;
    model->draw      = draw_model;
    model->release   = release_model;
    model->opaque    = !material.enable_blend;

    return model;
}

void unlit_update_uniforms(model_t *model, mat4_t mvp_matrix) {
    unlit_uniforms_t *uniforms = get_uniforms(model);
    uniforms->mvp_matrix = mvp_matrix;
}
