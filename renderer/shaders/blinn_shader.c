#include <math.h>
#include <stdlib.h>
#include "../core/api.h"
#include "blinn_shader.h"
#include "cache_helper.h"

/* low-level api */

static const float AMBIENT_INTENSITY = 0.4f;

static mat4_t get_model_matrix(blinn_attribs_t *attribs,
                               blinn_uniforms_t *uniforms) {
    if (uniforms->joint_matrices) {
        mat4_t joint_matrices[4];
        mat4_t skin_matrix;

        joint_matrices[0] = uniforms->joint_matrices[(int)attribs->joint.x];
        joint_matrices[1] = uniforms->joint_matrices[(int)attribs->joint.y];
        joint_matrices[2] = uniforms->joint_matrices[(int)attribs->joint.z];
        joint_matrices[3] = uniforms->joint_matrices[(int)attribs->joint.w];

        skin_matrix = mat4_combine(joint_matrices, attribs->weight);
        return mat4_mul_mat4(uniforms->model_matrix, skin_matrix);
    } else {
        return uniforms->model_matrix;
    }
}

static mat3_t get_normal_matrix(blinn_attribs_t *attribs,
                                blinn_uniforms_t *uniforms) {
    if (uniforms->joint_n_matrices) {
        mat3_t joint_n_matrices[4];
        mat3_t skin_n_matrix;

        joint_n_matrices[0] = uniforms->joint_n_matrices[(int)attribs->joint.x];
        joint_n_matrices[1] = uniforms->joint_n_matrices[(int)attribs->joint.y];
        joint_n_matrices[2] = uniforms->joint_n_matrices[(int)attribs->joint.z];
        joint_n_matrices[3] = uniforms->joint_n_matrices[(int)attribs->joint.w];

        skin_n_matrix = mat3_combine(joint_n_matrices, attribs->weight);
        return mat3_mul_mat3(uniforms->normal_matrix, skin_n_matrix);
    } else {
        return uniforms->normal_matrix;
    }
}

static vec4_t shadow_vertex_shader(blinn_attribs_t *attribs,
                                   blinn_varyings_t *varyings,
                                   blinn_uniforms_t *uniforms) {
    mat4_t model_matrix = get_model_matrix(attribs, uniforms);
    mat4_t light_vp_matrix = uniforms->light_vp_matrix;

    vec4_t input_position = vec4_from_vec3(attribs->position, 1);
    vec4_t world_position = mat4_mul_vec4(model_matrix, input_position);
    vec4_t depth_position = mat4_mul_vec4(light_vp_matrix, world_position);

    varyings->texcoord = attribs->texcoord;
    return depth_position;
}

static vec4_t common_vertex_shader(blinn_attribs_t *attribs,
                                   blinn_varyings_t *varyings,
                                   blinn_uniforms_t *uniforms) {
    mat4_t model_matrix = get_model_matrix(attribs, uniforms);
    mat3_t normal_matrix = get_normal_matrix(attribs, uniforms);
    mat4_t camera_vp_matrix = uniforms->camera_vp_matrix;
    mat4_t light_vp_matrix = uniforms->light_vp_matrix;

    vec4_t input_position = vec4_from_vec3(attribs->position, 1);
    vec4_t world_position = mat4_mul_vec4(model_matrix, input_position);
    vec4_t clip_position = mat4_mul_vec4(camera_vp_matrix, world_position);
    vec4_t depth_position = mat4_mul_vec4(light_vp_matrix, world_position);

    vec3_t input_normal = attribs->normal;
    vec3_t world_normal = mat3_mul_vec3(normal_matrix, input_normal);
    world_normal = vec3_normalize(world_normal);

    varyings->world_position = vec3_from_vec4(world_position);
    varyings->depth_position = vec3_from_vec4(depth_position);
    varyings->texcoord = attribs->texcoord;
    varyings->normal = world_normal;
    return clip_position;
}

vec4_t blinn_vertex_shader(void *attribs_, void *varyings_, void *uniforms_) {
    blinn_attribs_t *attribs = (blinn_attribs_t*)attribs_;
    blinn_varyings_t *varyings = (blinn_varyings_t*)varyings_;
    blinn_uniforms_t *uniforms = (blinn_uniforms_t*)uniforms_;

    if (uniforms->shadow_pass) {
        return shadow_vertex_shader(attribs, varyings, uniforms);
    } else {
        return common_vertex_shader(attribs, varyings, uniforms);
    }
}

static vec4_t shadow_fragment_shader(blinn_varyings_t *varyings,
                                     blinn_uniforms_t *uniforms,
                                     int *discard) {
    if (uniforms->alpha_cutoff > 0) {
        float alpha;
        if (uniforms->diffuse_map) {
            vec2_t texcoord = varyings->texcoord;
            alpha = texture_sample(uniforms->diffuse_map, texcoord).w;
        } else {
            alpha = uniforms->basecolor.w;
        }
        if (alpha < uniforms->alpha_cutoff) {
            *discard = 1;
        }
    }
    return vec4_new(0, 0, 0, 0);
}

static vec3_t get_basecolor(blinn_varyings_t *varyings,
                            blinn_uniforms_t *uniforms,
                            float *alpha) {
    if (uniforms->diffuse_map) {
        vec2_t texcoord = varyings->texcoord;
        vec4_t sample = texture_sample(uniforms->diffuse_map, texcoord);
        *alpha = sample.w;
        return vec3_from_vec4(sample);
    } else {
        *alpha = uniforms->basecolor.w;
        return vec3_from_vec4(uniforms->basecolor);
    }
}

static int is_in_shadow(blinn_varyings_t *varyings, blinn_uniforms_t *uniforms,
                        vec3_t normal_dir, vec3_t light_dir) {
    if (uniforms->shadow_map) {
        float u = (varyings->depth_position.x + 1) * 0.5f;
        float v = (varyings->depth_position.y + 1) * 0.5f;
        float d = (varyings->depth_position.z + 1) * 0.5f;

        float n_dot_l = vec3_dot(normal_dir, light_dir);
        float depth_bias = float_max(0.05f * (1 - n_dot_l), 0.005f);
        float current_depth = d - depth_bias;

        vec2_t texcoord = vec2_new(u, v);
        float closest_depth = texture_sample(uniforms->shadow_map, texcoord).x;

        return current_depth > closest_depth;
    } else {
        return 0;
    }
}

static vec4_t common_fragment_shader(blinn_varyings_t *varyings,
                                     blinn_uniforms_t *uniforms,
                                     int *discard) {
    float alpha;
    vec3_t basecolor = get_basecolor(varyings, uniforms, &alpha);
    if (uniforms->alpha_cutoff > 0 && alpha < uniforms->alpha_cutoff) {
        *discard = 1;
        return vec4_new(0, 0, 0, 0);
    } else {
        vec3_t light_dir = vec3_negate(uniforms->light_dir);
        vec3_t normal_dir = vec3_normalize(varyings->normal);
        int shadowed = is_in_shadow(varyings, uniforms, normal_dir, light_dir);

        vec3_t color = vec3_mul(basecolor, AMBIENT_INTENSITY);

        if (!shadowed) {
            float strength = vec3_dot(normal_dir, light_dir);
            if (strength > 0) {
                vec3_t diffuse = vec3_mul(basecolor, strength);
                color = vec3_add(color, diffuse);
            }
        }

        if (!shadowed && uniforms->specular_map) {
            vec3_t camera_pos = uniforms->camera_pos;
            vec3_t world_pos = varyings->world_position;
            vec3_t view_dir = vec3_normalize(vec3_sub(camera_pos, world_pos));
            vec3_t half_dir = vec3_normalize(vec3_add(light_dir, view_dir));

            float closeness = vec3_dot(half_dir, normal_dir);
            if (closeness > 0) {
                float strength = (float)pow(closeness, uniforms->shininess);
                texture_t *specular_map = uniforms->specular_map;
                vec2_t texcoord = varyings->texcoord;
                vec4_t sample = texture_sample(specular_map, texcoord);
                vec3_t specular = vec3_mul(vec3_from_vec4(sample), strength);
                color = vec3_add(color, specular);
            }
        }

        if (uniforms->emission_map) {
            texture_t *emission_map = uniforms->emission_map;
            vec2_t texcoord = varyings->texcoord;
            vec4_t sample = texture_sample(emission_map, texcoord);
            vec3_t emission = vec3_from_vec4(sample);
            color = vec3_add(color, emission);
        }

        return vec4_from_vec3(color, alpha);
    }
}

vec4_t blinn_fragment_shader(void *varyings_, void *uniforms_, int *discard) {
    blinn_varyings_t *varyings = (blinn_varyings_t*)varyings_;
    blinn_uniforms_t *uniforms = (blinn_uniforms_t*)uniforms_;

    if (uniforms->shadow_pass) {
        return shadow_fragment_shader(varyings, uniforms, discard);
    } else {
        return common_fragment_shader(varyings, uniforms, discard);
    }
}

/* high-level api */

static void update_model(model_t *model, perframe_t *perframe) {
    mat4_t model_matrix = model->transform;
    mat4_t normal_matrix = mat4_inverse_transpose(model_matrix);
    blinn_uniforms_t *uniforms;

    uniforms = (blinn_uniforms_t*)program_get_uniforms(model->program);
    uniforms->light_dir = perframe->light_dir;
    uniforms->camera_pos = perframe->camera_pos;
    uniforms->model_matrix = model_matrix;
    uniforms->normal_matrix = mat3_from_mat4(normal_matrix);
    uniforms->light_vp_matrix = mat4_mul_mat4(perframe->light_proj_matrix,
                                              perframe->light_view_matrix);
    uniforms->camera_vp_matrix = mat4_mul_mat4(perframe->camera_proj_matrix,
                                               perframe->camera_view_matrix);
    if (model->skeleton) {
        skeleton_t *skeleton = model->skeleton;
        skeleton_update_joints(skeleton, perframe->frame_time);
        uniforms->joint_matrices = skeleton_get_joint_matrices(skeleton);
        uniforms->joint_n_matrices = skeleton_get_normal_matrices(skeleton);
    }
    uniforms->shadow_map = perframe->shadow_map;
}

static void draw_model(model_t *model, framebuffer_t *framebuffer,
                       int shadow_pass) {
    program_t *program = model->program;
    mesh_t *mesh = model->mesh;
    int num_faces = mesh_get_num_faces(mesh);
    vertex_t *vertices = mesh_get_vertices(mesh);
    blinn_uniforms_t *uniforms;
    blinn_attribs_t *attribs;
    int i, j;

    uniforms = (blinn_uniforms_t*)program_get_uniforms(model->program);
    uniforms->shadow_pass = shadow_pass;
    for (i = 0; i < num_faces; i++) {
        for (j = 0; j < 3; j++) {
            vertex_t vertex = vertices[i * 3 + j];
            attribs = (blinn_attribs_t*)program_get_attribs(program, j);
            attribs->position = vertex.position;
            attribs->texcoord = vertex.texcoord;
            attribs->normal = vertex.normal;
            attribs->joint = vertex.joint;
            attribs->weight = vertex.weight;
        }
        graphics_draw_triangle(framebuffer, program);
    }
}

static void release_model(model_t *model) {
    blinn_uniforms_t *uniforms;
    uniforms = (blinn_uniforms_t*)program_get_uniforms(model->program);
    cache_release_texture(uniforms->diffuse_map);
    cache_release_texture(uniforms->specular_map);
    cache_release_texture(uniforms->emission_map);
    program_release(model->program);
    cache_release_skeleton(model->skeleton);
    cache_release_mesh(model->mesh);
    free(model);
}

model_t *blinn_create_model(const char *mesh, const char *skeleton,
                            mat4_t transform, blinn_material_t material) {
    int sizeof_attribs = sizeof(blinn_attribs_t);
    int sizeof_varyings = sizeof(blinn_varyings_t);
    int sizeof_uniforms = sizeof(blinn_uniforms_t);
    blinn_uniforms_t *uniforms;
    program_t *program;
    model_t *model;

    program = program_create(blinn_vertex_shader, blinn_fragment_shader,
                             sizeof_attribs, sizeof_varyings, sizeof_uniforms,
                             material.double_sided, material.enable_blend);

    uniforms = (blinn_uniforms_t*)program_get_uniforms(program);
    uniforms->basecolor = material.basecolor;
    uniforms->shininess = material.shininess;
    uniforms->diffuse_map = cache_acquire_texture(material.diffuse_map, 0);
    uniforms->specular_map = cache_acquire_texture(material.specular_map, 0);
    uniforms->emission_map = cache_acquire_texture(material.emission_map, 0);
    uniforms->alpha_cutoff = material.alpha_cutoff;

    model = (model_t*)malloc(sizeof(model_t));
    model->mesh      = cache_acquire_mesh(mesh);
    model->transform = transform;
    model->program   = program;
    model->skeleton  = cache_acquire_skeleton(skeleton);
    model->draw      = draw_model;
    model->update    = update_model;
    model->release   = release_model;
    model->opaque    = !material.enable_blend;

    return model;
}
