#include <stdlib.h>
#include "../core/api.h"
#include "skinned_shader.h"

/* low-level api */

static mat4_t get_skin_matrix(vec4_t joint_indices_, vec4_t joint_weights_,
                              mat4_t joint_matrices[256]) {
    mat4_t skin_matrix;
    int joint_indices[4];
    float joint_weights[4];
    int i, j, k;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            skin_matrix.m[i][j] = 0;
        }
    }

    joint_indices[0] = (int)joint_indices_.x;
    joint_indices[1] = (int)joint_indices_.y;
    joint_indices[2] = (int)joint_indices_.z;
    joint_indices[3] = (int)joint_indices_.w;

    joint_weights[0] = joint_weights_.x;
    joint_weights[1] = joint_weights_.y;
    joint_weights[2] = joint_weights_.z;
    joint_weights[3] = joint_weights_.w;

    for (k = 0; k < 4; k++) {
        if (joint_weights[k] > 0) {
            int joint_index = joint_indices[k];
            float joint_weight = joint_weights[k];
            mat4_t joint_matrix = joint_matrices[joint_index];
            for (i = 0; i < 4; i++) {
                for (j = 0; j < 4; j++) {
                    skin_matrix.m[i][j] += joint_weight * joint_matrix.m[i][j];
                }
            }
        }
    }

    return skin_matrix;
}

vec4_t skinned_vertex_shader(void *attribs_, void *varyings_, void *uniforms_) {
    skinned_attribs_t *attribs = (skinned_attribs_t*)attribs_;
    skinned_varyings_t *varyings = (skinned_varyings_t*)varyings_;
    skinned_uniforms_t *uniforms = (skinned_uniforms_t*)uniforms_;

    mat4_t skin_matrix = get_skin_matrix(attribs->joint, attribs->weight,
                                         uniforms->joint_matrices);

    vec4_t bind_pos = vec4_from_vec3(attribs->position, 1);
    vec4_t local_pos = mat4_mul_vec4(skin_matrix, bind_pos);
    vec4_t clip_pos = mat4_mul_vec4(uniforms->mvp_matrix, local_pos);
    varyings->texcoord = attribs->texcoord;
    return clip_pos;
}

vec4_t skinned_fragment_shader(void *varyings_, void *uniforms_) {
    skinned_varyings_t *varyings = (skinned_varyings_t*)varyings_;
    skinned_uniforms_t *uniforms = (skinned_uniforms_t*)uniforms_;

    if (uniforms->texture) {
        vec4_t sample = texture_sample(uniforms->texture, varyings->texcoord);
        return vec4_modulate(uniforms->factor, sample);
    } else {
        return uniforms->factor;
    }
}

/* high-level api */

static skinned_uniforms_t *get_uniforms(model_t *model) {
    return (skinned_uniforms_t*)program_get_uniforms(model->program);
}

static void release_model(model_t *model) {
    skinned_uniforms_t *uniforms = get_uniforms(model);
    if (uniforms->texture) {
        texture_release(uniforms->texture);
    }
    program_release(model->program);
    mesh_release(model->mesh);
    free(model);
}

static void draw_model(model_t *model, framebuffer_t *framebuffer) {
    program_t *program = model->program;
    mesh_t *mesh = model->mesh;
    int num_faces = mesh_get_num_faces(mesh);
    skinned_attribs_t *attribs;
    int i, j;

    for (i = 0; i < num_faces; i++) {
        for (j = 0; j < 3; j++) {
            vertex_t vertex = mesh_get_vertex(mesh, i, j);
            attribs = (skinned_attribs_t*)program_get_attribs(program, j);
            attribs->position = vertex.position;
            attribs->texcoord = vertex.texcoord;
            attribs->joint = vertex.joint;
            attribs->weight = vertex.weight;
        }
        graphics_draw_triangle(framebuffer, program);
    }
}

model_t *skinned_create_model(const char *mesh, mat4_t transform,
                              skinned_material_t material) {
    int sizeof_attribs = sizeof(skinned_attribs_t);
    int sizeof_varyings = sizeof(skinned_varyings_t);
    int sizeof_uniforms = sizeof(skinned_uniforms_t);
    skinned_uniforms_t *uniforms;
    program_t *program;
    model_t *model;

    program = program_create(skinned_vertex_shader, skinned_fragment_shader,
                             sizeof_attribs, sizeof_varyings, sizeof_uniforms,
                             material.double_sided, material.enable_blend);
    uniforms = (skinned_uniforms_t*)program_get_uniforms(program);
    uniforms->factor = material.factor;
    if (material.texture) {
        uniforms->texture = texture_from_file(material.texture);
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

void skinned_update_uniforms(model_t *model, mat4_t mvp_matrix,
                             skeleton_t *skeleton) {
    skinned_uniforms_t *uniforms = get_uniforms(model);
    uniforms->mvp_matrix = mvp_matrix;
    skeleton_dump_matrices(skeleton, uniforms->joint_matrices);
}
