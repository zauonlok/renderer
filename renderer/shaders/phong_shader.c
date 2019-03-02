#include <math.h>
#include <stdlib.h>
#include "../core/api.h"
#include "phong_shader.h"

/* low-level api */

vec4_t phong_vertex_shader(void *attribs_, void *varyings_, void *uniforms_) {
    phong_attribs_t *attribs = (phong_attribs_t*)attribs_;
    phong_varyings_t *varyings = (phong_varyings_t*)varyings_;
    phong_uniforms_t *uniforms = (phong_uniforms_t*)uniforms_;

    vec4_t local_pos = vec4_from_vec3(attribs->position, 1);
    vec4_t world_pos = mat4_mul_vec4(uniforms->model_matrix, local_pos);
    vec4_t clip_pos = mat4_mul_vec4(uniforms->viewproj_matrix, world_pos);

    vec3_t local_normal = attribs->normal;
    vec3_t world_normal = mat3_mul_vec3(uniforms->normal_matrix, local_normal);

    varyings->position = vec3_from_vec4(world_pos);
    varyings->texcoord = attribs->texcoord;
    varyings->normal = world_normal;
    return clip_pos;
}

static vec3_t reflect_light(vec3_t light, vec3_t normal) {
    return vec3_sub(vec3_mul(normal, 2 * vec3_dot(light, normal)), light);
}

vec4_t phong_fragment_shader(void *varyings_, void *uniforms_) {
    phong_varyings_t *varyings = (phong_varyings_t*)varyings_;
    phong_uniforms_t *uniforms = (phong_uniforms_t*)uniforms_;

    vec2_t texcoord = varyings->texcoord;
    vec3_t normal = vec3_normalize(varyings->normal);
    vec3_t light_dir = vec3_negate(uniforms->light_dir);

    vec3_t color = vec3_new(0, 0, 0);
    float alpha = 1;

    if (uniforms->diffuse) {
        vec4_t sample = texture_sample(uniforms->diffuse, texcoord);
        vec3_t albedo = vec3_from_vec4(sample);

        float strength = float_max(vec3_dot(normal, light_dir), 0);
        vec3_t diffuse = vec3_mul(albedo, strength + uniforms->ambient);

        color = vec3_add(color, diffuse);
        alpha = sample.w;
    }

    if (uniforms->specular) {
        vec3_t world_pos = varyings->position;
        vec3_t camera_pos = uniforms->camera_pos;
        vec3_t view_dir = vec3_normalize(vec3_sub(camera_pos, world_pos));
        vec3_t reflected_dir = reflect_light(light_dir, normal);

        float closeness = vec3_dot(reflected_dir, view_dir);
        if (closeness > 0) {
            float strength = (float)pow(closeness, uniforms->shininess);
            vec4_t sample = texture_sample(uniforms->specular, texcoord);
            vec3_t specular = vec3_mul(vec3_from_vec4(sample), strength);
            color = vec3_add(color, specular);
        }
    }

    if (uniforms->emission) {
        vec4_t sample = texture_sample(uniforms->emission, texcoord);
        vec3_t emission = vec3_from_vec4(sample);
        color = vec3_add(color, emission);
    }

    return vec4_from_vec3(color, alpha);
}

/* high-level api */

static phong_uniforms_t *get_uniforms(model_t *model) {
    return (phong_uniforms_t*)program_get_uniforms(model->program);
}

static void release_model(model_t *model) {
    phong_uniforms_t *uniforms = get_uniforms(model);
    if (uniforms->emission) {
        texture_release(uniforms->emission);
    }
    if (uniforms->diffuse) {
        texture_release(uniforms->diffuse);
    }
    if (uniforms->specular) {
        texture_release(uniforms->specular);
    }
    program_release(model->program);
    mesh_release(model->mesh);
    free(model);
}

model_t *phong_create_model(const char *mesh, mat4_t transform,
                            phong_material_t material) {
    int sizeof_attribs = sizeof(phong_attribs_t);
    int sizeof_varyings = sizeof(phong_varyings_t);
    int sizeof_uniforms = sizeof(phong_uniforms_t);
    phong_uniforms_t *uniforms;
    program_t *program;
    model_t *model;

    program = program_create(phong_vertex_shader, phong_fragment_shader,
                             sizeof_attribs, sizeof_varyings, sizeof_uniforms,
                             material.double_sided, material.enable_blend);
    uniforms = (phong_uniforms_t*)program_get_uniforms(program);
    uniforms->ambient = material.ambient;
    uniforms->shininess = material.shininess;
    if (material.emission) {
        uniforms->emission = texture_from_file(material.emission);
    }
    if (material.diffuse) {
        uniforms->diffuse = texture_from_file(material.diffuse);
    }
    if (material.specular) {
        uniforms->specular = texture_from_file(material.specular);
    }

    model = (model_t*)malloc(sizeof(model_t));
    model->mesh      = mesh_load(mesh);
    model->transform = transform;
    model->program   = program;
    model->release   = release_model;
    model->opaque    = !material.enable_blend;

    return model;
}

void phong_update_uniforms(
        model_t *model, vec3_t light_dir, vec3_t camera_pos,
        mat4_t model_matrix, mat3_t normal_matrix, mat4_t viewproj_matrix) {
    phong_uniforms_t *uniforms = get_uniforms(model);
    uniforms->light_dir = light_dir;
    uniforms->camera_pos = camera_pos;
    uniforms->model_matrix = model_matrix;
    uniforms->normal_matrix = normal_matrix;
    uniforms->viewproj_matrix = viewproj_matrix;
}

void phong_draw_model(model_t *model, framebuffer_t *framebuffer) {
    program_t *program = model->program;
    mesh_t *mesh = model->mesh;
    int num_faces = mesh_get_num_faces(mesh);
    phong_attribs_t *attribs;
    int i, j;

    for (i = 0; i < num_faces; i++) {
        for (j = 0; j < 3; j++) {
            attribs = (phong_attribs_t*)program_get_attribs(program, j);
            attribs->position = mesh_get_position(mesh, i, j);
            attribs->texcoord = mesh_get_texcoord(mesh, i, j);
            attribs->normal = mesh_get_normal(mesh, i, j);
        }
        graphics_draw_triangle(framebuffer, program);
    }
}
