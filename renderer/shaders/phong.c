#include "phong.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "../core/apis.h"

/* low-level apis */

vec4_t phong_vertex_shader(void *attribs_, void *varyings_, void *uniforms_) {
    phong_attribs_t *attribs = (phong_attribs_t*)attribs_;
    phong_varyings_t *varyings = (phong_varyings_t*)varyings_;
    phong_uniforms_t *uniforms = (phong_uniforms_t*)uniforms_;

    vec4_t local_pos = vec4_from_vec3(attribs->local_pos, 1);
    vec4_t world_pos = mat4_mul_vec4(uniforms->model_matrix, local_pos);
    vec4_t clip_pos = mat4_mul_vec4(uniforms->viewproj_matrix, world_pos);
    varyings->world_pos = vec3_from_vec4(world_pos);
    varyings->texcoord = attribs->texcoord;
    return clip_pos;
}

/*
 * for normal transformation, see
 * https://github.com/ssloy/tinyrenderer/wiki/Lesson-5:-Moving-the-camera
 */
static vec3_t transform_normal(vec4_t normal, mat4_t normal_matrix) {
    normal = vec4_new(normal.x * 2 - 1, normal.y * 2 - 1, normal.z * 2 - 1, 0);
    normal = mat4_mul_vec4(normal_matrix, normal);
    return vec3_normalize(vec3_from_vec4(normal));
}

/*
 * for the reflect function, see
 * http://docs.gl/sl4/reflect
 */
static vec3_t reflect_light(vec3_t light, vec3_t normal) {
    return vec3_sub(light, vec3_mul(normal, 2 * vec3_dot(light, normal)));
}

static float max_float(float a, float b) {
    return a > b ? a : b;
}

vec4_t phong_fragment_shader(void *varyings_, void *uniforms_) {
    phong_varyings_t *varyings = (phong_varyings_t*)varyings_;
    phong_uniforms_t *uniforms = (phong_uniforms_t*)uniforms_;

    /* sampling */
    vec2_t uv = varyings->texcoord;
    vec4_t normal_ = texture_sample(uniforms->normal_map, uv.x, uv.y);
    vec4_t diffuse_ = texture_sample(uniforms->diffuse_map, uv.x, uv.y);
    vec4_t specular_ = texture_sample(uniforms->specular_map, uv.x, uv.y);

    /* ambient */
    vec3_t ambient = vec3_new(0.1f, 0.1f, 0.1f);

    /* diffuse */
    vec3_t light_dir = vec3_normalize(uniforms->light_dir);
    vec3_t normal = transform_normal(normal_, uniforms->model_it_matrix);
    float diffuse_factor = max_float(-vec3_dot(light_dir, normal), 0);
    vec3_t diffuse = vec3_mul(vec3_from_vec4(diffuse_), diffuse_factor);

    /* specular */
    vec3_t reflected = reflect_light(vec3_negative(light_dir), normal);
    vec3_t view_offset = vec3_sub(varyings->world_pos, uniforms->camera_pos);
    vec3_t view_dir = vec3_normalize(view_offset);
    float closeness = max_float(-vec3_dot(reflected, view_dir), 0);
    float specular_factor = (float)pow(closeness, 16) * 0.6f;
    vec3_t specular = vec3_mul(vec3_from_vec4(specular_), specular_factor);

    /* assembling */
    float color_r = ambient.x + diffuse.x + specular.x;
    float color_g = ambient.y + diffuse.y + specular.y;
    float color_b = ambient.z + diffuse.z + specular.z;
    return vec4_new(color_r, color_g, color_b, 1);
}

program_t *phong_create_program(void) {
    program_t *program;
    int i;

    program = (program_t*)malloc(sizeof(program_t));
    for (i = 0; i < 3; i++) {
        program->attribs[i] = malloc(sizeof(phong_attribs_t));
        memset(program->attribs[i], 0, sizeof(phong_attribs_t));
    }
    for (i = 0; i < 4; i++) {
        program->varyings[i] = malloc(sizeof(phong_varyings_t));
        memset(program->varyings[i], 0, sizeof(phong_varyings_t));
    }
    program->uniforms = malloc(sizeof(phong_uniforms_t));
    memset(program->uniforms, 0, sizeof(phong_uniforms_t));

    program->vertex_shader   = phong_vertex_shader;
    program->fragment_shader = phong_fragment_shader;
    program->sizeof_varyings = sizeof(phong_varyings_t);

    return program;
}

void phong_release_program(program_t *program) {
    int i;
    for (i = 0; i < 3; i++) {
        free(program->attribs[i]);
    }
    for (i = 0; i < 4; i++) {
        free(program->varyings[i]);
    }
    free(program->uniforms);
    free(program);
}

/* high-level apis */

model_t *phong_create_model(mesh_t *mesh, image_t *normal_map,
                            image_t *diffuse_map, image_t *specular_map) {
    phong_uniforms_t *uniforms;
    program_t *program;
    model_t *model;

    program = phong_create_program();
    uniforms = (phong_uniforms_t*)program->uniforms;
    uniforms->normal_map   = texture_from_image(normal_map);
    uniforms->diffuse_map  = texture_from_image(diffuse_map);
    uniforms->specular_map = texture_from_image(specular_map);

    model = (model_t*)malloc(sizeof(model_t));
    model->mesh    = mesh;
    model->program = program;

    return model;
}

void phong_release_model(model_t *model) {
    phong_uniforms_t *uniforms = phong_get_uniforms(model);
    texture_release(uniforms->normal_map);
    texture_release(uniforms->diffuse_map);
    texture_release(uniforms->specular_map);

    phong_release_program(model->program);
    free(model);
}

phong_uniforms_t *phong_get_uniforms(model_t *model) {
    return (phong_uniforms_t*)model->program->uniforms;
}

void phong_draw_model(rendertarget_t *rendertarget, model_t *model) {
    program_t *program = model->program;
    mesh_t *mesh = model->mesh;
    int num_faces = mesh_get_num_faces(mesh);
    int i, j;

    for (i = 0; i < num_faces; i++) {
        for (j = 0; j < 3; j++) {
            phong_attribs_t *attribs = (phong_attribs_t*)program->attribs[j];
            attribs->local_pos = mesh_get_position(mesh, i, j);
            attribs->texcoord = mesh_get_texcoord(mesh, i, j);
        }
        graphics_draw_triangle(rendertarget, program);
    }
}
