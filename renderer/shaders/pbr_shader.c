#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "../core/api.h"
#include "cache_helper.h"
#include "pbr_shader.h"

/* low-level api */

static mat4_t get_model_matrix(pbr_attribs_t *attribs,
                               pbr_uniforms_t *uniforms) {
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

static mat3_t get_normal_matrix(pbr_attribs_t *attribs,
                                pbr_uniforms_t *uniforms) {
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

static vec4_t shadow_vertex_shader(pbr_attribs_t *attribs,
                                   pbr_varyings_t *varyings,
                                   pbr_uniforms_t *uniforms) {
    mat4_t model_matrix = get_model_matrix(attribs, uniforms);
    mat4_t light_vp_matrix = uniforms->light_vp_matrix;

    vec4_t input_position = vec4_from_vec3(attribs->position, 1);
    vec4_t world_position = mat4_mul_vec4(model_matrix, input_position);
    vec4_t depth_position = mat4_mul_vec4(light_vp_matrix, world_position);

    varyings->texcoord = attribs->texcoord;
    return depth_position;
}

static vec4_t common_vertex_shader(pbr_attribs_t *attribs,
                                   pbr_varyings_t *varyings,
                                   pbr_uniforms_t *uniforms) {
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

    if (uniforms->normal_map) {
        mat3_t tangent_matrix = mat3_from_mat4(model_matrix);
        vec3_t input_tangent = vec3_from_vec4(attribs->tangent);
        vec3_t world_tangent = mat3_mul_vec3(tangent_matrix, input_tangent);
        vec3_t world_bitangent;

        world_normal = vec3_normalize(world_normal);
        world_tangent = vec3_normalize(world_tangent);
        world_bitangent = vec3_cross(world_normal, world_tangent);
        world_bitangent = vec3_mul(world_bitangent, attribs->tangent.w);

        varyings->world_normal = world_normal;
        varyings->world_tangent = world_tangent;
        varyings->world_bitangent = world_bitangent;
    } else {
        varyings->world_normal = vec3_normalize(world_normal);
    }

    varyings->world_position = vec3_from_vec4(world_position);
    varyings->depth_position = vec3_from_vec4(depth_position);
    varyings->texcoord = attribs->texcoord;
    return clip_position;
}

vec4_t pbr_vertex_shader(void *attribs_, void *varyings_, void *uniforms_) {
    pbr_attribs_t *attribs = (pbr_attribs_t*)attribs_;
    pbr_varyings_t *varyings = (pbr_varyings_t*)varyings_;
    pbr_uniforms_t *uniforms = (pbr_uniforms_t*)uniforms_;

    if (uniforms->shadow_pass) {
        return shadow_vertex_shader(attribs, varyings, uniforms);
    } else {
        return common_vertex_shader(attribs, varyings, uniforms);
    }
}

static vec4_t shadow_fragment_shader(pbr_varyings_t *varyings,
                                     pbr_uniforms_t *uniforms,
                                     int *discard) {
    if (uniforms->alpha_cutoff > 0) {
        float alpha;
        if (uniforms->workflow == METALNESS_WORKFLOW) {
            alpha = uniforms->basecolor_factor.w;
            if (uniforms->basecolor_map) {
                vec2_t texcoord = varyings->texcoord;
                alpha *= texture_sample(uniforms->basecolor_map, texcoord).w;
            }
        } else {
            alpha = uniforms->diffuse_factor.w;
            if (uniforms->diffuse_map) {
                vec2_t texcoord = varyings->texcoord;
                alpha *= texture_sample(uniforms->diffuse_map, texcoord).w;
            }
        }
        if (alpha < uniforms->alpha_cutoff) {
            *discard = 1;
        }
    }
    return vec4_new(0, 0, 0, 0);
}

static vec3_t get_view_dir(pbr_varyings_t *varyings,
                           pbr_uniforms_t *uniforms) {
    vec3_t camera_pos = uniforms->camera_pos;
    vec3_t world_pos = varyings->world_position;
    return vec3_normalize(vec3_sub(camera_pos, world_pos));
}

static vec3_t get_normal_dir(pbr_varyings_t *varyings,
                             pbr_uniforms_t *uniforms,
                             vec3_t view_dir) {
    vec3_t normal_dir;
    if (uniforms->normal_map) {
        vec2_t texcoord = varyings->texcoord;
        vec4_t sample = texture_sample(uniforms->normal_map, texcoord);
        vec3_t tangent_normal = vec3_new(sample.x * 2 - 1,
                                         sample.y * 2 - 1,
                                         sample.z * 2 - 1);
        mat3_t tbn_matrix = mat3_from_cols(varyings->world_tangent,
                                           varyings->world_bitangent,
                                           varyings->world_normal);
        vec3_t world_normal = mat3_mul_vec3(tbn_matrix, tangent_normal);
        normal_dir = vec3_normalize(world_normal);
    } else {
        normal_dir = vec3_normalize(varyings->world_normal);
    }
    if (vec3_dot(normal_dir, view_dir) < 0) {
        return vec3_negate(normal_dir);
    } else {
        return normal_dir;
    }
}

static vec3_t get_incident_dir(vec3_t normal_dir, vec3_t view_dir) {
    float n_dot_v = vec3_dot(normal_dir, view_dir);
    return vec3_sub(vec3_mul(normal_dir, 2 * n_dot_v), view_dir);
}

static vec3_t get_ibl_shade(ibldata_t *ibldata, float roughness,
                            vec3_t normal_dir, vec3_t view_dir,
                            vec3_t diffuse_color, vec3_t specular_color_) {
    cubemap_t *diffuse_map = ibldata->diffuse_map;
    vec4_t diffuse_sample = cubemap_clamp_sample(diffuse_map, normal_dir);
    vec3_t diffuse_light = vec3_from_vec4(diffuse_sample);
    vec3_t diffuse_shade = vec3_modulate(diffuse_light, diffuse_color);

    float n_dot_v = vec3_dot(normal_dir, view_dir);
    vec2_t brdf_texcoord = vec2_new(n_dot_v, roughness);
    vec4_t brdf_sample = texture_clamp_sample(ibldata->brdf_lut, brdf_texcoord);
    float specular_scale = brdf_sample.x;
    float specular_bias = brdf_sample.y;

    float specular_r = specular_color_.x * specular_scale + specular_bias;
    float specular_g = specular_color_.y * specular_scale + specular_bias;
    float specular_b = specular_color_.z * specular_scale + specular_bias;
    vec3_t specular_color = vec3_new(specular_r, specular_g, specular_b);

    vec3_t incident_dir = get_incident_dir(normal_dir, view_dir);
    int specular_lod = (int)(roughness * (ibldata->mip_level - 1));
    cubemap_t *specular_map = ibldata->specular_maps[specular_lod];
    vec4_t specular_sample = cubemap_clamp_sample(specular_map, incident_dir);
    vec3_t specular_light = vec3_from_vec4(specular_sample);
    vec3_t specular_shade = vec3_modulate(specular_light, specular_color);

    return vec3_add(diffuse_shade, specular_shade);
}

static int is_in_shadow(pbr_varyings_t *varyings,
                        pbr_uniforms_t *uniforms,
                        float n_dot_l) {
    if (uniforms->shadow_map) {
        float u = (varyings->depth_position.x + 1) * 0.5f;
        float v = (varyings->depth_position.y + 1) * 0.5f;
        float d = (varyings->depth_position.z + 1) * 0.5f;

        float depth_bias = float_max(0.05f * (1 - n_dot_l), 0.005f);
        float current_depth = d - depth_bias;
        vec2_t texcoord = vec2_new(u, v);
        float closest_depth = texture_sample(uniforms->shadow_map, texcoord).x;

        return current_depth > closest_depth;
    } else {
        return 0;
    }
}

/*
 * for normal distribution function, see
 * Microfacet Models for Refraction through Rough Surfaces
 */
static float get_distribution(float n_dot_h, float alpha2) {
    float n_dot_h_2 = n_dot_h * n_dot_h;
    float factor = n_dot_h_2 * (alpha2 - 1) + 1;
    return alpha2 / (PI * factor * factor);
}

/*
 * for geometry function, see
 * Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs
 */
static float get_visibility(float n_dot_v, float n_dot_l, float alpha2) {
    float n_dot_v_2 = n_dot_v * n_dot_v;
    float n_dot_l_2 = n_dot_l * n_dot_l;
    float ggx_v = n_dot_l * (float)sqrt(n_dot_v_2 * (1 - alpha2) + alpha2);
    float ggx_l = n_dot_v * (float)sqrt(n_dot_l_2 * (1 - alpha2) + alpha2);
    return 0.5f / (ggx_v + ggx_l);
}

/*
 * for fresnel approximation, see
 * An Inexpensive BRDF Model for Physically-based Rendering
 */
static vec3_t get_fresnel(float v_dot_h, vec3_t reflectance0) {
    vec3_t reflectance90 = vec3_new(1, 1, 1);
    return vec3_lerp(reflectance0, reflectance90, (float)pow(1 - v_dot_h, 5));
}

static vec3_t get_dir_shade(vec3_t light_dir, float roughness,
                            vec3_t normal_dir, vec3_t view_dir,
                            vec3_t diffuse_color, vec3_t specular_color) {
    float n_dot_l = vec3_dot(normal_dir, light_dir);
    float n_dot_v = vec3_dot(normal_dir, view_dir);
    if (n_dot_l > 0 && n_dot_v > 0) {
        vec3_t half_dir = vec3_normalize(vec3_add(light_dir, view_dir));
        float n_dot_h = float_max(vec3_dot(normal_dir, half_dir), 0);
        float v_dot_h = float_max(vec3_dot(view_dir, half_dir), 0);

        float alpha = roughness * roughness;
        float alpha2 = alpha * alpha;

        float d_term = get_distribution(n_dot_h, alpha2);
        float v_term = get_visibility(n_dot_v, n_dot_l, alpha2);
        vec3_t f_term = get_fresnel(v_dot_h, specular_color);

        vec3_t diffuse_brdf = vec3_div(diffuse_color, PI);
        vec3_t specular_brdf = vec3_mul(f_term, v_term * d_term);

        float combined_r = (1 - f_term.x) * diffuse_brdf.x + specular_brdf.x;
        float combined_g = (1 - f_term.y) * diffuse_brdf.y + specular_brdf.y;
        float combined_b = (1 - f_term.z) * diffuse_brdf.z + specular_brdf.z;
        vec3_t combined_brdf = vec3_new(combined_r, combined_g, combined_b);

        return vec3_mul(combined_brdf, n_dot_l);
    } else {
        return vec3_new(0, 0, 0);
    }
}

static vec3_t linear_to_srgb(vec3_t color) {
    float r = (float)pow(color.x, 1 / 2.2);
    float g = (float)pow(color.y, 1 / 2.2);
    float b = (float)pow(color.z, 1 / 2.2);
    return vec3_new(r, g, b);
}

static float max_component(vec3_t v) {
    return v.x > v.y && v.x > v.z ? v.x : (v.y > v.z ? v.y : v.z);
}

static const vec3_t DIELECTRIC_SPECULAR = {0.04f, 0.04f, 0.04f};

static vec4_t common_fragment_shader(pbr_varyings_t *varyings,
                                     pbr_uniforms_t *uniforms,
                                     int *discard) {
    vec2_t texcoord = varyings->texcoord;
    vec3_t diffuse, specular;
    float alpha, roughness;
    if (uniforms->workflow == METALNESS_WORKFLOW) {
        vec3_t basecolor;
        float metalness;

        basecolor = vec3_from_vec4(uniforms->basecolor_factor);
        alpha = uniforms->basecolor_factor.w;
        if (uniforms->basecolor_map) {
            vec4_t sample = texture_sample(uniforms->basecolor_map, texcoord);
            basecolor = vec3_modulate(basecolor, vec3_from_vec4(sample));
            alpha *= sample.w;
        }

        metalness = uniforms->metalness_factor;
        if (uniforms->metalness_map) {
            vec4_t sample = texture_sample(uniforms->metalness_map, texcoord);
            metalness *= sample.x;
        }

        roughness = uniforms->roughness_factor;
        if (uniforms->roughness_map) {
            vec4_t sample = texture_sample(uniforms->roughness_map, texcoord);
            roughness *= sample.x;
        }

        diffuse = vec3_mul(basecolor, (1 - 0.04f) * (1 - metalness));
        specular = vec3_lerp(DIELECTRIC_SPECULAR, basecolor, metalness);
    } else {
        float glossiness;

        diffuse = vec3_from_vec4(uniforms->diffuse_factor);
        alpha = uniforms->diffuse_factor.w;
        if (uniforms->diffuse_map) {
            vec4_t sample = texture_sample(uniforms->diffuse_map, texcoord);
            diffuse = vec3_modulate(diffuse, vec3_from_vec4(sample));
            alpha *= sample.w;
        }

        specular = uniforms->specular_factor;
        if (uniforms->specular_map) {
            vec4_t sample = texture_sample(uniforms->specular_map, texcoord);
            specular = vec3_modulate(specular, vec3_from_vec4(sample));
        }

        glossiness = uniforms->glossiness_factor;
        if (uniforms->glossiness_map) {
            vec4_t sample = texture_sample(uniforms->glossiness_map, texcoord);
            glossiness *= sample.x;
        }

        diffuse = vec3_mul(diffuse, 1 - max_component(specular));
        roughness = 1 - glossiness;
    }

    if (uniforms->alpha_cutoff > 0 && alpha < uniforms->alpha_cutoff) {
        *discard = 1;
        return vec4_new(0, 0, 0, 0);
    } else {
        vec3_t light_dir = vec3_negate(uniforms->light_dir);
        vec3_t view_dir = get_view_dir(varyings, uniforms);
        vec3_t normal_dir = get_normal_dir(varyings, uniforms, view_dir);
        float n_dot_l = vec3_dot(normal_dir, light_dir);

        vec3_t color = vec3_new(0, 0, 0);

        if (uniforms->ambient_intensity > 0 && uniforms->ibldata) {
            float ambient_intensity = uniforms->ambient_intensity;
            vec3_t shade = get_ibl_shade(uniforms->ibldata, roughness,
                                         normal_dir, view_dir,
                                         diffuse, specular);
            color = vec3_add(color, vec3_mul(shade, ambient_intensity));
        }

        if (uniforms->punctual_intensity > 0 && n_dot_l > 0) {
            float punctual_intensity = uniforms->punctual_intensity;
            if (!is_in_shadow(varyings, uniforms, n_dot_l)) {
                vec3_t shade = get_dir_shade(light_dir, roughness,
                                             normal_dir, view_dir,
                                             diffuse, specular);
                color = vec3_add(color, vec3_mul(shade, punctual_intensity));
            }
        }

        if (uniforms->occlusion_map) {
            vec4_t sample = texture_sample(uniforms->occlusion_map, texcoord);
            float occlusion = sample.x;
            color = vec3_mul(color, occlusion);
        }

        if (uniforms->emission_map) {
            vec4_t sample = texture_sample(uniforms->emission_map, texcoord);
            vec3_t emission = vec3_from_vec4(sample);
            color = vec3_add(color, emission);
        }

        return vec4_from_vec3(linear_to_srgb(color), alpha);
    }
}

vec4_t pbr_fragment_shader(void *varyings_, void *uniforms_, int *discard) {
    pbr_varyings_t *varyings = (pbr_varyings_t*)varyings_;
    pbr_uniforms_t *uniforms = (pbr_uniforms_t*)uniforms_;

    if (uniforms->shadow_pass) {
        return shadow_fragment_shader(varyings, uniforms, discard);
    } else {
        return common_fragment_shader(varyings, uniforms, discard);
    }
}

/* high-level api */

static void update_model(model_t *model, framedata_t *framedata) {
    float ambient_intensity = framedata->ambient_intensity;
    float punctual_intensity = framedata->punctual_intensity;
    skeleton_t *skeleton = model->skeleton;
    mat4_t model_matrix = model->transform;
    mat3_t normal_matrix;
    mat4_t *joint_matrices;
    mat3_t *joint_n_matrices;
    pbr_uniforms_t *uniforms;

    if (skeleton) {
        skeleton_update_joints(skeleton, framedata->frame_time);
        joint_matrices = skeleton_get_joint_matrices(skeleton);
        joint_n_matrices = skeleton_get_normal_matrices(skeleton);
        if (model->attached >= 0) {
            mat4_t node_matrix = joint_matrices[model->attached];
            model_matrix = mat4_mul_mat4(model_matrix, node_matrix);
            joint_matrices = NULL;
            joint_n_matrices = NULL;
        }
    } else {
        joint_matrices = NULL;
        joint_n_matrices = NULL;
    }
    normal_matrix = mat3_inverse_transpose(mat3_from_mat4(model_matrix));

    uniforms = (pbr_uniforms_t*)program_get_uniforms(model->program);
    uniforms->light_dir = framedata->light_dir;
    uniforms->camera_pos = framedata->camera_pos;
    uniforms->model_matrix = model_matrix;
    uniforms->normal_matrix = normal_matrix;
    uniforms->light_vp_matrix = mat4_mul_mat4(framedata->light_proj_matrix,
                                              framedata->light_view_matrix);
    uniforms->camera_vp_matrix = mat4_mul_mat4(framedata->camera_proj_matrix,
                                               framedata->camera_view_matrix);
    uniforms->joint_matrices = joint_matrices;
    uniforms->joint_n_matrices = joint_n_matrices;
    uniforms->ambient_intensity = float_clamp(ambient_intensity, 0, 5);
    uniforms->punctual_intensity = float_clamp(punctual_intensity, 0, 5);
    uniforms->shadow_map = framedata->shadow_map;
}

static void draw_model(model_t *model, framebuffer_t *framebuffer,
                       int shadow_pass) {
    mesh_t *mesh = model->mesh;
    int num_faces = mesh_get_num_faces(mesh);
    vertex_t *vertices = mesh_get_vertices(mesh);
    program_t *program = model->program;
    pbr_uniforms_t *uniforms;
    pbr_attribs_t *attribs;
    int i, j;

    uniforms = (pbr_uniforms_t*)program_get_uniforms(model->program);
    uniforms->shadow_pass = shadow_pass;
    for (i = 0; i < num_faces; i++) {
        for (j = 0; j < 3; j++) {
            vertex_t vertex = vertices[i * 3 + j];
            attribs = (pbr_attribs_t*)program_get_attribs(program, j);
            attribs->position = vertex.position;
            attribs->texcoord = vertex.texcoord;
            attribs->normal = vertex.normal;
            attribs->tangent = vertex.tangent;
            attribs->joint = vertex.joint;
            attribs->weight = vertex.weight;
        }
        graphics_draw_triangle(framebuffer, program);
    }
}

static void release_model(model_t *model) {
    pbr_uniforms_t *uniforms;
    uniforms = (pbr_uniforms_t*)program_get_uniforms(model->program);
    cache_release_texture(uniforms->basecolor_map);
    cache_release_texture(uniforms->metalness_map);
    cache_release_texture(uniforms->roughness_map);
    cache_release_texture(uniforms->diffuse_map);
    cache_release_texture(uniforms->specular_map);
    cache_release_texture(uniforms->glossiness_map);
    cache_release_texture(uniforms->normal_map);
    cache_release_texture(uniforms->occlusion_map);
    cache_release_texture(uniforms->emission_map);
    cache_release_ibldata(uniforms->ibldata);
    program_release(model->program);
    cache_release_skeleton(model->skeleton);
    cache_release_mesh(model->mesh);
    free(model);
}

static model_t *create_model(const char *mesh, const char *skeleton,
                             int attached, mat4_t transform,
                             int double_sided, int enable_blend) {
    int sizeof_attribs = sizeof(pbr_attribs_t);
    int sizeof_varyings = sizeof(pbr_varyings_t);
    int sizeof_uniforms = sizeof(pbr_uniforms_t);
    program_t *program;
    model_t *model;

    program = program_create(pbr_vertex_shader, pbr_fragment_shader,
                             sizeof_attribs, sizeof_varyings, sizeof_uniforms,
                             double_sided, enable_blend);

    model = (model_t*)malloc(sizeof(model_t));
    model->mesh = cache_acquire_mesh(mesh);
    model->skeleton = cache_acquire_skeleton(skeleton);
    model->attached = attached;
    model->program = program;
    model->transform = transform;
    model->sortdata.opaque = !enable_blend;
    model->sortdata.distance = 0;
    model->draw = draw_model;
    model->update = update_model;
    model->release = release_model;

    return model;
}

model_t *pbrm_create_model(const char *mesh, const char *skeleton,
                           int attached, mat4_t transform,
                           pbrm_material_t material, const char *env_name) {
    pbr_uniforms_t *uniforms;
    model_t *model;

    assert(material.metalness_factor >= 0 && material.metalness_factor <= 1);
    assert(material.roughness_factor >= 0 && material.roughness_factor <= 1);

    model = create_model(mesh, skeleton, attached, transform,
                         material.double_sided, material.enable_blend);

    uniforms = (pbr_uniforms_t*)program_get_uniforms(model->program);
    uniforms->basecolor_factor = material.basecolor_factor;
    uniforms->metalness_factor = material.metalness_factor;
    uniforms->roughness_factor = material.roughness_factor;
    uniforms->basecolor_map = cache_acquire_texture(material.basecolor_map, 1);
    uniforms->metalness_map = cache_acquire_texture(material.metalness_map, 0);
    uniforms->roughness_map = cache_acquire_texture(material.roughness_map, 0);
    uniforms->normal_map = cache_acquire_texture(material.normal_map, 0);
    uniforms->occlusion_map = cache_acquire_texture(material.occlusion_map, 0);
    uniforms->emission_map = cache_acquire_texture(material.emission_map, 1);
    uniforms->ibldata = cache_acquire_ibldata(env_name);
    uniforms->alpha_cutoff = material.alpha_cutoff;
    uniforms->workflow = METALNESS_WORKFLOW;

    return model;
}

model_t *pbrs_create_model(const char *mesh, const char *skeleton,
                           int attached, mat4_t transform,
                           pbrs_material_t material, const char *env_name) {
    pbr_uniforms_t *uniforms;
    model_t *model;

    assert(material.glossiness_factor >= 0 && material.glossiness_factor <= 1);

    model = create_model(mesh, skeleton, attached, transform,
                         material.double_sided, material.enable_blend);

    uniforms = (pbr_uniforms_t*)program_get_uniforms(model->program);
    uniforms->diffuse_factor = material.diffuse_factor;
    uniforms->specular_factor = material.specular_factor;
    uniforms->glossiness_factor = material.glossiness_factor;
    uniforms->diffuse_map = cache_acquire_texture(material.diffuse_map, 1);
    uniforms->specular_map = cache_acquire_texture(material.specular_map, 1);
    uniforms->glossiness_map = cache_acquire_texture(material.glossiness_map, 0);
    uniforms->normal_map = cache_acquire_texture(material.normal_map, 0);
    uniforms->occlusion_map = cache_acquire_texture(material.occlusion_map, 0);
    uniforms->emission_map = cache_acquire_texture(material.emission_map, 1);
    uniforms->ibldata = cache_acquire_ibldata(env_name);
    uniforms->alpha_cutoff = material.alpha_cutoff;
    uniforms->workflow = SPECULAR_WORKFLOW;

    return model;
}
