#include "phong_shader.h"
#include <math.h>
#include <stdlib.h>
#include "../core/apis.h"

/*
 * for half lambert, see
 * https://developer.valvesoftware.com/wiki/Half_Lambert
 */
static const int USE_HALF_LAMBERT = 1;

/* low-level apis */

vec4_t phong_vertex_shader(void *attribs_, void *varyings_, void *uniforms_) {
    phong_attribs_t *attribs = (phong_attribs_t*)attribs_;
    phong_varyings_t *varyings = (phong_varyings_t*)varyings_;
    phong_uniforms_t *uniforms = (phong_uniforms_t*)uniforms_;

    vec4_t local_pos = vec4_from_vec3(attribs->position, 1);
    vec4_t world_pos = mat4_mul_vec4(uniforms->model_matrix, local_pos);
    vec4_t clip_pos = mat4_mul_vec4(uniforms->viewproj_matrix, world_pos);

    vec4_t local_normal = vec4_from_vec3(attribs->normal, 0);
    mat4_t normal_matrix = uniforms->model_it_matrix;
    vec4_t world_normal = mat4_mul_vec4(normal_matrix, local_normal);

    varyings->position = vec3_from_vec4(world_pos);
    varyings->texcoord = attribs->texcoord;
    varyings->normal = vec3_from_vec4(world_normal);
    return clip_pos;
}

static vec4_t calculate_emission(vec2_t texcoord, phong_uniforms_t *uniforms) {
    if (uniforms->emission_texture) {
        vec4_t factor = uniforms->emission_factor;
        vec4_t color = texture_sample(uniforms->emission_texture, texcoord);
        return vec4_modulate(factor, color);
    } else {
        return uniforms->emission_factor;
    }
}

static vec4_t calculate_diffuse(vec2_t texcoord, phong_uniforms_t *uniforms) {
    if (uniforms->diffuse_texture) {
        vec4_t factor = uniforms->diffuse_factor;
        vec4_t color = texture_sample(uniforms->diffuse_texture, texcoord);
        return vec4_modulate(factor, color);
    } else {
        return uniforms->diffuse_factor;
    }
}

static vec4_t calculate_specular(vec2_t texcoord, phong_uniforms_t *uniforms) {
    if (uniforms->specular_texture) {
        vec4_t factor = uniforms->specular_factor;
        vec4_t color = texture_sample(uniforms->specular_texture, texcoord);
        return vec4_modulate(factor, color);
    } else {
        return uniforms->specular_factor;
    }
}

static float max_float(float a, float b) {
    return a > b ? a : b;
}

static float calculate_diffuse_strength(vec3_t light_dir, vec3_t normal) {
    float l_dot_n = -vec3_dot(light_dir, normal);
    if (USE_HALF_LAMBERT) {
        return (l_dot_n * 0.5f + 0.5f) * (l_dot_n * 0.5f + 0.5f);
    } else {
        return max_float(l_dot_n, 0);
    }
}

/*
 * for the reflect function, see
 * http://docs.gl/sl4/reflect
 */
static vec3_t reflect_light(vec3_t light, vec3_t normal) {
    return vec3_sub(light, vec3_mul(normal, 2 * vec3_dot(light, normal)));
}

static float calculate_specular_strength(vec3_t light_dir, vec3_t eye_dir,
                                         vec3_t normal, float shininess) {
    vec3_t reflected = reflect_light(light_dir, normal);
    float base = max_float(-vec3_dot(reflected, eye_dir), 0);
    return (float)pow(base, shininess);
}

vec4_t phong_fragment_shader(void *varyings_, void *uniforms_) {
    phong_varyings_t *varyings = (phong_varyings_t*)varyings_;
    phong_uniforms_t *uniforms = (phong_uniforms_t*)uniforms_;

    vec4_t ambient = uniforms->ambient_factor;
    vec4_t emission = calculate_emission(varyings->texcoord, uniforms);
    vec4_t diffuse_ = calculate_diffuse(varyings->texcoord, uniforms);
    vec4_t specular_ = calculate_specular(varyings->texcoord, uniforms);

    vec3_t light_dir = vec3_normalize(uniforms->light_dir);
    vec3_t normal = vec3_normalize(varyings->normal);
    vec3_t world_pos = varyings->position;
    vec3_t eye_dir = vec3_normalize(vec3_sub(world_pos, uniforms->camera_pos));

    float d_strength = calculate_diffuse_strength(light_dir, normal);
    float s_strength = calculate_specular_strength(light_dir, eye_dir, normal,
                                                   uniforms->shininess);
    vec3_t diffuse = vec3_mul(vec3_from_vec4(diffuse_), d_strength);
    vec3_t specular = vec3_mul(vec3_from_vec4(specular_), s_strength);

    float color_r = ambient.x + emission.x + diffuse.x + specular.x;
    float color_g = ambient.y + emission.y + diffuse.y + specular.y;
    float color_b = ambient.z + emission.z + diffuse.z + specular.z;

    return vec4_new(color_r, color_g, color_b, 1);
}

/* high-level apis */

model_t *phong_create_model(const char *mesh_filename, mat4_t transform,
                            phong_material_t material) {
    int sizeof_attribs = sizeof(phong_attribs_t);
    int sizeof_varyings = sizeof(phong_varyings_t);
    int sizeof_uniforms = sizeof(phong_uniforms_t);
    phong_uniforms_t *uniforms;
    program_t *program;
    model_t *model;

    program = program_create(phong_vertex_shader, phong_fragment_shader,
                             sizeof_attribs, sizeof_varyings, sizeof_uniforms);
    uniforms = (phong_uniforms_t*)program->uniforms;
    uniforms->ambient_factor = material.ambient_factor;
    uniforms->emission_factor = material.emission_factor;
    uniforms->diffuse_factor = material.diffuse_factor;
    uniforms->specular_factor = material.specular_factor;
    uniforms->shininess = material.shininess;
    if (material.emission_texture) {
        const char *emission_filename = material.emission_texture;
        uniforms->emission_texture = texture_from_file(emission_filename);
    }
    if (material.diffuse_texture) {
        const char *diffuse_filename = material.diffuse_texture;
        uniforms->diffuse_texture = texture_from_file(diffuse_filename);
    }
    if (material.specular_texture) {
        const char *specular_filename = material.specular_texture;
        uniforms->specular_texture = texture_from_file(specular_filename);
    }

    model = (model_t*)malloc(sizeof(model_t));
    model->transform = transform;
    model->mesh      = mesh_load(mesh_filename);
    model->program   = program;

    return model;
}

void phong_release_model(model_t *model) {
    phong_uniforms_t *uniforms = phong_get_uniforms(model);
    if (uniforms->emission_texture) {
        texture_release(uniforms->emission_texture);
    }
    if (uniforms->diffuse_texture) {
        texture_release(uniforms->diffuse_texture);
    }
    if (uniforms->specular_texture) {
        texture_release(uniforms->specular_texture);
    }
    program_release(model->program);
    mesh_release(model->mesh);
    free(model);
}

phong_uniforms_t *phong_get_uniforms(model_t *model) {
    return (phong_uniforms_t*)model->program->uniforms;
}

void phong_draw_model(model_t *model, framebuffer_t *framebuffer) {
    program_t *program = model->program;
    mesh_t *mesh = model->mesh;
    int num_faces = mesh_get_num_faces(mesh);
    phong_attribs_t *attribs;
    int i, j;

    for (i = 0; i < num_faces; i++) {
        for (j = 0; j < 3; j++) {
            attribs = (phong_attribs_t*)program->attribs[j];
            attribs->position = mesh_get_position(mesh, i, j);
            attribs->texcoord = mesh_get_texcoord(mesh, i, j);
            attribs->normal = mesh_get_normal(mesh, i, j);
        }
        graphics_draw_triangle(framebuffer, program);
    }
}
