#include <math.h>
#include <stdlib.h>
#include "../core/api.h"
#include "metalness_shader.h"

/* low-level api */

static mat3_t build_tbn_matrix(metalness_attribs_t *attribs,
                               metalness_uniforms_t *uniforms) {
    mat3_t model_matrix = mat3_from_mat4(uniforms->model_matrix);
    mat3_t normal_matrix = mat3_from_mat4(uniforms->model_it_matrix);

    vec3_t local_normal = attribs->normal;
    vec3_t world_normal_ = mat3_mul_vec3(normal_matrix, local_normal);
    vec3_t world_normal = vec3_normalize(world_normal_);

    vec3_t local_tangent = vec3_from_vec4(attribs->tangent);
    vec3_t world_tangent_ = mat3_mul_vec3(model_matrix, local_tangent);
    vec3_t world_tangent = vec3_normalize(world_tangent_);

    float handedness = attribs->tangent.w;
    vec3_t world_bitangent_ = vec3_cross(world_normal, world_tangent);
    vec3_t world_bitangent = vec3_mul(world_bitangent_, handedness);

    return mat3_from_cols(world_tangent, world_bitangent, world_normal);
}

vec4_t metalness_vertex_shader(void *attribs_, void *varyings_,
                               void *uniforms_) {
    metalness_attribs_t *attribs = (metalness_attribs_t*)attribs_;
    metalness_varyings_t *varyings = (metalness_varyings_t*)varyings_;
    metalness_uniforms_t *uniforms = (metalness_uniforms_t*)uniforms_;

    vec4_t local_pos = vec4_from_vec3(attribs->position, 1);
    vec4_t world_pos = mat4_mul_vec4(uniforms->model_matrix, local_pos);
    vec4_t clip_pos = mat4_mul_vec4(uniforms->viewproj_matrix, world_pos);

    varyings->position = vec3_from_vec4(world_pos);
    varyings->texcoord = attribs->texcoord;
    varyings->tbn_matrix = build_tbn_matrix(attribs, uniforms);
    return clip_pos;
}

static vec3_t get_basecolor(vec2_t texcoord, metalness_uniforms_t *uniforms) {
    if (uniforms->basecolor_texture) {
        vec3_t factor = uniforms->basecolor_factor;
        vec4_t color = texture_sample(uniforms->basecolor_texture, texcoord);
        return vec3_modulate(factor, vec3_from_vec4(color));
    } else {
        return uniforms->basecolor_factor;
    }
}

static float get_metallic(vec2_t texcoord, metalness_uniforms_t *uniforms) {
    if (uniforms->metallic_texture) {
        vec4_t sample = texture_sample(uniforms->metallic_texture, texcoord);
        return uniforms->metallic_factor * sample.x;
    } else {
        return uniforms->metallic_factor;
    }
}

static float get_roughness(vec2_t texcoord, metalness_uniforms_t *uniforms) {
    if (uniforms->roughness_texture) {
        vec4_t sample = texture_sample(uniforms->roughness_texture, texcoord);
        return uniforms->roughness_factor * sample.x;
    } else {
        return uniforms->roughness_factor;
    }
}

static vec3_t get_normal(vec2_t texcoord, mat3_t tbn_matrix,
                         metalness_uniforms_t *uniforms) {
    if (uniforms->normal_texture) {
        vec4_t sample = texture_sample(uniforms->normal_texture, texcoord);
        float normal_x = sample.x * 2 - 1;
        float normal_y = sample.y * 2 - 1;
        float normal_z = sample.z * 2 - 1;
        vec3_t normal = vec3_new(normal_x, normal_y, normal_z);
        return vec3_normalize(mat3_mul_vec3(tbn_matrix, normal));
    } else {
        float normal_x = tbn_matrix.m[0][2];
        float normal_y = tbn_matrix.m[1][2];
        float normal_z = tbn_matrix.m[2][2];
        vec3_t normal = vec3_new(normal_x, normal_y, normal_z);
        return vec3_normalize(normal);
    }
}

static float get_occlusion(vec2_t texcoord, metalness_uniforms_t *uniforms) {
    if (uniforms->occlusion_texture) {
        vec4_t sample = texture_sample(uniforms->occlusion_texture, texcoord);
        return sample.x;
    } else {
        return 1;
    }
}

static vec3_t get_emission(vec2_t texcoord, metalness_uniforms_t *uniforms) {
    if (uniforms->emissive_texture) {
        vec4_t sample = texture_sample(uniforms->emissive_texture, texcoord);
        return vec3_from_vec4(sample);
    } else {
        return vec3_new(0, 0, 0);
    }
}

static vec3_t get_diffuse(vec3_t basecolor, float metallic) {
    return vec3_mul(basecolor, (1 - 0.04f) * (1 - metallic));
}

static vec3_t get_specular(vec3_t basecolor, float metallic) {
    return vec3_lerp(vec3_new(0.04f, 0.04f, 0.04f), basecolor, metallic);
}

static float max_float(float a, float b) {
    return a > b ? a : b;
}

static float abs_float(float f) {
    return (f >= 0) ? f : -f;
}

static float get_distribution(float n_dot_h, float alpha2) {
    float factor = (n_dot_h * n_dot_h) * (alpha2 - 1) + 1;
    return alpha2 / (PI * factor * factor);
}

static float get_visibility(float n_dot_v, float n_dot_l, float alpha2) {
    float n_dot_v_2 = n_dot_v * n_dot_v;
    float n_dot_l_2 = n_dot_l * n_dot_l;
    float term_v = n_dot_l * (float)sqrt(n_dot_v_2 * (1 - alpha2) + alpha2);
    float term_l = n_dot_v * (float)sqrt(n_dot_l_2 * (1 - alpha2) + alpha2);
    return 0.5f / (term_v + term_l);
}

static vec3_t get_fresnel(float v_dot_h, vec3_t reflectance0) {
    vec3_t reflectance90 = vec3_new(1, 1, 1);
    return vec3_lerp(reflectance0, reflectance90, (float)pow(1 - v_dot_h, 5));
}

vec4_t metalness_fragment_shader(void *varyings_, void *uniforms_) {
    metalness_varyings_t *varyings = (metalness_varyings_t*)varyings_;
    metalness_uniforms_t *uniforms = (metalness_uniforms_t*)uniforms_;

    vec2_t texcoord = varyings->texcoord;
    vec3_t basecolor = get_basecolor(texcoord, uniforms);
    float metallic = get_metallic(texcoord, uniforms);
    float roughness = get_roughness(texcoord, uniforms);
    vec3_t normal = get_normal(texcoord, varyings->tbn_matrix, uniforms);
    float occlusion = get_occlusion(texcoord, uniforms);
    vec3_t emission = get_emission(texcoord, uniforms);

    vec3_t diffuse_color = get_diffuse(basecolor, metallic);
    vec3_t specular_color = get_specular(basecolor, metallic);

    vec3_t world_pos = varyings->position;
    vec3_t camera_pos = uniforms->camera_pos;
    vec3_t light_dir = vec3_negate(uniforms->light_dir);
    vec3_t view_dir = vec3_normalize(vec3_sub(camera_pos, world_pos));
    vec3_t half_dir = vec3_normalize(vec3_add(light_dir, view_dir));

    float n_dot_v = abs_float(vec3_dot(normal, view_dir)) + EPSILON;
    float n_dot_l = max_float(vec3_dot(normal, light_dir), 0);
    float n_dot_h = max_float(vec3_dot(normal, half_dir), 0);
    float v_dot_h = max_float(vec3_dot(view_dir, half_dir), 0);

    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float d_term = get_distribution(n_dot_h, alpha2);
    float v_term = get_visibility(n_dot_v, n_dot_l, alpha2);
    vec3_t f_term = get_fresnel(v_dot_h, specular_color);

    vec3_t diffuse_brdf = vec3_div(diffuse_color, PI);
    vec3_t specular_brdf = vec3_mul(f_term, v_term * d_term);

    vec3_t inv_f_term = vec3_sub(vec3_new(1, 1, 1), f_term);
    vec3_t diffuse_term = vec3_modulate(inv_f_term, diffuse_brdf);
    vec3_t combined_brdf = vec3_add(diffuse_term, specular_brdf);

    vec3_t light_color = vec3_mul(vec3_new(1, 1, 1), n_dot_l);
    vec3_t color = vec3_modulate(combined_brdf, light_color);

    return vec4_from_vec3(color, 1);
}

/* high-level api */

model_t *metalness_create_model(const char *mesh_filename, mat4_t transform,
                                metalness_material_t material) {
    int sizeof_attribs = sizeof(metalness_attribs_t);
    int sizeof_varyings = sizeof(metalness_varyings_t);
    int sizeof_uniforms = sizeof(metalness_uniforms_t);
    metalness_uniforms_t *uniforms;
    program_t *program;
    model_t *model;

    program = program_create(metalness_vertex_shader, metalness_fragment_shader,
                             sizeof_attribs, sizeof_varyings, sizeof_uniforms);
    uniforms = (metalness_uniforms_t*)program_get_uniforms(program);
    uniforms->basecolor_factor = material.basecolor_factor;
    uniforms->metallic_factor = material.metallic_factor;
    uniforms->roughness_factor = material.roughness_factor;
    if (material.basecolor_texture) {
        const char *basecolor_filename = material.basecolor_texture;
        uniforms->basecolor_texture = texture_from_file(basecolor_filename);
    }
    if (material.metallic_texture) {
        const char *metallic_filename = material.metallic_texture;
        uniforms->metallic_texture = texture_from_file(metallic_filename);
    }
    if (material.roughness_texture) {
        const char *roughness_filename = material.roughness_texture;
        uniforms->roughness_texture = texture_from_file(roughness_filename);
    }
    if (material.normal_texture) {
        const char *normal_filename = material.normal_texture;
        uniforms->normal_texture = texture_from_file(normal_filename);
    }
    if (material.occlusion_texture) {
        const char *occlusion_filename = material.occlusion_texture;
        uniforms->occlusion_texture = texture_from_file(occlusion_filename);
    }
    if (material.emissive_texture) {
        const char *emissive_filename = material.emissive_texture;
        uniforms->emissive_texture = texture_from_file(emissive_filename);
    }

    model = (model_t*)malloc(sizeof(model_t));
    model->transform = transform;
    model->mesh      = mesh_load(mesh_filename);
    model->program   = program;

    return model;
}

void metalness_release_model(model_t *model) {
    metalness_uniforms_t *uniforms = metalness_get_uniforms(model);
    if (uniforms->basecolor_texture) {
        texture_release(uniforms->basecolor_texture);
    }
    if (uniforms->metallic_texture) {
        texture_release(uniforms->metallic_texture);
    }
    if (uniforms->roughness_texture) {
        texture_release(uniforms->roughness_texture);
    }
    if (uniforms->normal_texture) {
        texture_release(uniforms->normal_texture);
    }
    if (uniforms->occlusion_texture) {
        texture_release(uniforms->occlusion_texture);
    }
    if (uniforms->emissive_texture) {
        texture_release(uniforms->emissive_texture);
    }
    program_release(model->program);
    mesh_release(model->mesh);
    free(model);
}

metalness_uniforms_t *metalness_get_uniforms(model_t *model) {
    return (metalness_uniforms_t*)program_get_uniforms(model->program);
}

void metalness_draw_model(model_t *model, framebuffer_t *framebuffer) {
    program_t *program = model->program;
    mesh_t *mesh = model->mesh;
    int num_faces = mesh_get_num_faces(mesh);
    metalness_attribs_t *attribs;
    int i, j;

    for (i = 0; i < num_faces; i++) {
        for (j = 0; j < 3; j++) {
            attribs = (metalness_attribs_t*)program_get_attribs(program, j);
            attribs->position = mesh_get_position(mesh, i, j);
            attribs->texcoord = mesh_get_texcoord(mesh, i, j);
            attribs->normal = mesh_get_normal(mesh, i, j);
            attribs->tangent = mesh_get_tangent(mesh, i, j);
        }
        graphics_draw_triangle(framebuffer, program);
    }
}
