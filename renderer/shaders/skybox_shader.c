#include <stdlib.h>
#include "../core/api.h"
#include "cache_helper.h"
#include "skybox_shader.h"

/* low-level api */

vec4_t skybox_vertex_shader(void *attribs_, void *varyings_, void *uniforms_) {
    skybox_attribs_t *attribs = (skybox_attribs_t*)attribs_;
    skybox_varyings_t *varyings = (skybox_varyings_t*)varyings_;
    skybox_uniforms_t *uniforms = (skybox_uniforms_t*)uniforms_;

    vec4_t local_pos = vec4_from_vec3(attribs->position, 1);
    vec4_t clip_pos = mat4_mul_vec4(uniforms->vp_matrix, local_pos);
    clip_pos.z = clip_pos.w * (1 - EPSILON);

    varyings->direction = attribs->position;
    return clip_pos;
}

vec4_t skybox_fragment_shader(void *varyings_, void *uniforms_,
                              int *discard, int backface) {
    skybox_varyings_t *varyings = (skybox_varyings_t*)varyings_;
    skybox_uniforms_t *uniforms = (skybox_uniforms_t*)uniforms_;

    UNUSED_VAR(discard);
    UNUSED_VAR(backface);
    return cubemap_sample(uniforms->skybox, varyings->direction);
}

/* high-level api */

static void update_model(model_t *model, perframe_t *perframe) {
    mat4_t view_matrix = perframe->camera_view_matrix;
    mat4_t proj_matrix = perframe->camera_proj_matrix;
    skybox_uniforms_t *uniforms;

    /* remove translation */
    view_matrix.m[0][3] = 0;
    view_matrix.m[1][3] = 0;
    view_matrix.m[2][3] = 0;

    uniforms = (skybox_uniforms_t*)program_get_uniforms(model->program);
    uniforms->vp_matrix = mat4_mul_mat4(proj_matrix, view_matrix);
}

static void draw_model(model_t *model, framebuffer_t *framebuffer,
                       int shadow_pass) {
    if (!shadow_pass) {
        mesh_t *mesh = model->mesh;
        int num_faces = mesh_get_num_faces(mesh);
        vertex_t *vertices = mesh_get_vertices(mesh);
        program_t *program = model->program;
        skybox_attribs_t *attribs;
        int i, j;

        for (i = 0; i < num_faces; i++) {
            for (j = 0; j < 3; j++) {
                vertex_t vertex = vertices[i * 3 + j];
                attribs = (skybox_attribs_t*)program_get_attribs(program, j);
                attribs->position = vertex.position;
            }
            graphics_draw_triangle(framebuffer, program);
        }
    }
}

static void release_model(model_t *model) {
    skybox_uniforms_t *uniforms;
    uniforms = (skybox_uniforms_t*)program_get_uniforms(model->program);
    cache_release_skybox(uniforms->skybox);
    program_release(model->program);
    cache_release_mesh(model->mesh);
    free(model);
}

model_t *skybox_create_model(const char *skybox_name, int blur_level) {
    int sizeof_attribs = sizeof(skybox_attribs_t);
    int sizeof_varyings = sizeof(skybox_varyings_t);
    int sizeof_uniforms = sizeof(skybox_uniforms_t);
    skybox_uniforms_t *uniforms;
    program_t *program;
    model_t *model;

    program = program_create(skybox_vertex_shader, skybox_fragment_shader,
                             sizeof_attribs, sizeof_varyings, sizeof_uniforms,
                             1, 0);

    uniforms = (skybox_uniforms_t*)program_get_uniforms(program);
    uniforms->skybox = cache_acquire_skybox(skybox_name, blur_level);

    model = (model_t*)malloc(sizeof(model_t));
    model->mesh = cache_acquire_mesh("common/box.obj");
    model->program = program;
    model->transform = mat4_identity();
    model->skeleton = NULL;
    model->attached = -1;
    model->opaque = 1;
    model->distance = 0;
    model->update = update_model;
    model->draw = draw_model;
    model->release = release_model;

    return model;
}
