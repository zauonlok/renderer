#include <math.h>
#include <stdlib.h>
#include <string.h>
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
    varyings->clip_position = clip_position;
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

typedef struct {
    vec3_t diffuse;
    vec3_t specular;
    float alpha;
    float roughness;
    vec3_t normal;
    float occlusion;
    vec3_t emission;
} material_t;

static material_t get_pbrm_material(pbr_uniforms_t *uniforms, vec2_t texcoord) {
    vec3_t diffuse, specular, basecolor;
    float alpha, roughness, metalness;
    material_t material;

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
    specular = vec3_lerp(vec3_new(0.04f, 0.04f, 0.04f), basecolor, metalness);

    memset(&material, 0, sizeof(material_t));
    material.diffuse = diffuse;
    material.specular = specular;
    material.alpha = alpha;
    material.roughness = roughness;
    return material;
}

static float max_component(vec3_t v) {
    return v.x > v.y && v.x > v.z ? v.x : (v.y > v.z ? v.y : v.z);
}

static material_t get_pbrs_material(pbr_uniforms_t *uniforms, vec2_t texcoord) {
    vec3_t diffuse, specular;
    float alpha, roughness, glossiness;
    material_t material;

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

    memset(&material, 0, sizeof(material_t));
    material.diffuse = diffuse;
    material.specular = specular;
    material.alpha = alpha;
    material.roughness = roughness;
    return material;
}

static vec3_t get_normal_dir(pbr_varyings_t *varyings,
                             pbr_uniforms_t *uniforms,
                             int backface) {
    vec3_t normal_dir;
    if (uniforms->normal_map) {
        vec4_t sample = texture_sample(uniforms->normal_map,
                                       varyings->texcoord);
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
    return backface ? vec3_negate(normal_dir) : normal_dir;
}

static material_t get_pixel_material(pbr_varyings_t *varyings,
                                     pbr_uniforms_t *uniforms,
                                     int backface) {
    vec2_t texcoord = varyings->texcoord;
    material_t material;

    if (uniforms->workflow == METALNESS_WORKFLOW) {
        material = get_pbrm_material(uniforms, texcoord);
    } else {
        material = get_pbrs_material(uniforms, texcoord);
    }

    material.normal = get_normal_dir(varyings, uniforms, backface);

    if (uniforms->occlusion_map) {
        vec4_t sample = texture_sample(uniforms->occlusion_map, texcoord);
        material.occlusion = sample.x;
    } else {
        material.occlusion = 1;
    }

    if (uniforms->emission_map) {
        vec4_t sample = texture_sample(uniforms->emission_map, texcoord);
        material.emission = vec3_from_vec4(sample);
    } else {
        material.emission = vec3_new(0, 0, 0);
    }

    return material;
}

static vec3_t get_incident_dir(vec3_t normal_dir, vec3_t view_dir) {
    float n_dot_v = vec3_dot(normal_dir, view_dir);
    return vec3_sub(vec3_mul(normal_dir, 2 * n_dot_v), view_dir);
}

static vec3_t get_ibl_shade(material_t material, ibldata_t *ibldata,
                            vec3_t normal_dir, vec3_t view_dir) {
    vec3_t diffuse_color = vec3_mul(material.diffuse, material.occlusion);
    cubemap_t *diffuse_map = ibldata->diffuse_map;
    vec4_t diffuse_sample = cubemap_clamp_sample(diffuse_map, normal_dir);
    vec3_t diffuse_light = vec3_from_vec4(diffuse_sample);
    vec3_t diffuse_shade = vec3_modulate(diffuse_light, diffuse_color);

    float n_dot_v = vec3_dot(normal_dir, view_dir);
    vec2_t lut_texcoord = vec2_new(n_dot_v, material.roughness);
    vec4_t lut_sample = texture_clamp_sample(ibldata->brdf_lut, lut_texcoord);
    float specular_scale = lut_sample.x;
    float specular_bias = lut_sample.y;

    float specular_r = material.specular.x * specular_scale + specular_bias;
    float specular_g = material.specular.y * specular_scale + specular_bias;
    float specular_b = material.specular.z * specular_scale + specular_bias;
    vec3_t specular_color = vec3_new(specular_r, specular_g, specular_b);

    vec3_t incident_dir = get_incident_dir(normal_dir, view_dir);
    float max_mip_level = (float)(ibldata->mip_levels - 1);
    int specular_lod = (int)(material.roughness * max_mip_level + 0.5f);
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
 * for visibility function, see
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
static vec3_t get_fresnel(float v_dot_h, vec3_t fresnel0) {
    float factor = (float)pow(1 - v_dot_h, 5);
    float fresnel90 = float_saturate(max_component(fresnel0) * 50);
    float fresnel_r = fresnel0.x + (fresnel90 - fresnel0.x) * factor;
    float fresnel_g = fresnel0.y + (fresnel90 - fresnel0.y) * factor;
    float fresnel_b = fresnel0.z + (fresnel90 - fresnel0.z) * factor;
    return vec3_new(fresnel_r, fresnel_g, fresnel_b);
}

static vec3_t get_dir_shade(material_t material, vec3_t light_dir,
                            vec3_t normal_dir, vec3_t view_dir) {
    float n_dot_l = vec3_dot(normal_dir, light_dir);
    float n_dot_v = vec3_dot(normal_dir, view_dir);
    if (n_dot_l > 0 && n_dot_v > 0) {
        vec3_t half_dir = vec3_normalize(vec3_add(light_dir, view_dir));
        float n_dot_h = float_max(vec3_dot(normal_dir, half_dir), 0);
        float v_dot_h = float_max(vec3_dot(view_dir, half_dir), 0);

        float alpha_roughness = material.roughness * material.roughness;
        float alpha2 = alpha_roughness * alpha_roughness;

        float d_term = get_distribution(n_dot_h, alpha2);
        float v_term = get_visibility(n_dot_v, n_dot_l, alpha2);
        vec3_t f_term = get_fresnel(v_dot_h, material.specular);

        vec3_t diffuse_lobe = vec3_div(material.diffuse, PI);
        vec3_t specular_lobe = vec3_mul(f_term, v_term * d_term);

        float combined_r = (1 - f_term.x) * diffuse_lobe.x + specular_lobe.x;
        float combined_g = (1 - f_term.y) * diffuse_lobe.y + specular_lobe.y;
        float combined_b = (1 - f_term.z) * diffuse_lobe.z + specular_lobe.z;
        vec3_t combined_lobe = vec3_new(combined_r, combined_g, combined_b);

        return vec3_mul(combined_lobe, n_dot_l);
    } else {
        return vec3_new(0, 0, 0);
    }
}

static vec3_t get_view_dir(pbr_varyings_t *varyings, pbr_uniforms_t *uniforms) {
    vec3_t camera_pos = uniforms->camera_pos;
    vec3_t world_pos = varyings->world_position;
    return vec3_normalize(vec3_sub(camera_pos, world_pos));
}

static vec4_t linear_to_srgb(vec3_t color, float alpha) {
    float r = float_linear2srgb(float_aces(color.x));
    float g = float_linear2srgb(float_aces(color.y));
    float b = float_linear2srgb(float_aces(color.z));
    return vec4_new(r, g, b, alpha);
}

#define NUM_EDGES 5
#define EDGE_SPACE 0.15f
#define EDGE_START (1 - EDGE_SPACE * 0.5f)
#define EDGE_END (EDGE_SPACE * (NUM_EDGES - 0.5f))

static int above_layer_edge(int edge, vec2_t coord) {
    float offset = EDGE_SPACE * (float)edge;
    vec2_t start = vec2_new(EDGE_START - offset, 0);
    vec2_t end = vec2_new(EDGE_END - offset, 1);
    return vec2_edge(start, end, coord) > 0;
}

static vec4_t get_layer_color(int layer, material_t material) {
    float alpha = material.alpha;
    if (layer == 1) {
        return linear_to_srgb(material.diffuse, alpha);
    } else if (layer == 2) {
        return linear_to_srgb(material.specular, alpha);
    } else if (layer == 3) {
        float roughness = material.roughness;
        return vec4_new(roughness, roughness, roughness, alpha);
    } else if (layer == 4) {
        float occlusion = material.occlusion;
        return vec4_new(occlusion, occlusion, occlusion, alpha);
    } else {
        float normal_x = material.normal.x * 0.5f + 0.5f;
        float normal_y = material.normal.y * 0.5f + 0.5f;
        float normal_z = material.normal.z * 0.5f + 0.5f;
        return vec4_new(normal_x, normal_y, normal_z, alpha);
    }
}

static vec2_t get_normalized_coord(vec4_t clip_coord) {
    float x = clip_coord.x / clip_coord.w * 0.5f + 0.5f;
    float y = clip_coord.y / clip_coord.w * 0.5f + 0.5f;
    return vec2_new(x, y);
}

static vec4_t common_fragment_shader(pbr_varyings_t *varyings,
                                     pbr_uniforms_t *uniforms,
                                     int *discard,
                                     int backface) {
    material_t material = get_pixel_material(varyings, uniforms, backface);
    vec2_t coord = get_normalized_coord(varyings->clip_position);
    if (uniforms->alpha_cutoff > 0 && material.alpha < uniforms->alpha_cutoff) {
        *discard = 1;
        return vec4_new(0, 0, 0, 0);
    } else if (uniforms->layer_view > 0) {
        return get_layer_color(uniforms->layer_view, material);
    } else if (uniforms->layer_view == 0 && !above_layer_edge(0, coord)) {
        int edge;
        for (edge = 1; edge < NUM_EDGES; edge++) {
            if (above_layer_edge(edge, coord)) {
                break;
            }
        }
        return get_layer_color(edge, material);
    } else {
        vec3_t view_dir = get_view_dir(varyings, uniforms);
        vec3_t light_dir = vec3_negate(uniforms->light_dir);
        vec3_t normal_dir = material.normal;
        float n_dot_l = vec3_dot(normal_dir, light_dir);
        vec3_t color = material.emission;

        if (uniforms->ambient_intensity > 0 && uniforms->ibldata) {
            float intensity = uniforms->ambient_intensity;
            vec3_t shade = get_ibl_shade(material, uniforms->ibldata,
                                         normal_dir, view_dir);
            color = vec3_add(color, vec3_mul(shade, intensity));
        }

        if (uniforms->punctual_intensity > 0 && n_dot_l > 0) {
            float intensity = uniforms->punctual_intensity;
            if (!is_in_shadow(varyings, uniforms, n_dot_l)) {
                vec3_t shade = get_dir_shade(material, light_dir,
                                             normal_dir, view_dir);
                color = vec3_add(color, vec3_mul(shade, intensity));
            }
        }

        return linear_to_srgb(color, material.alpha);
    }
}

vec4_t pbr_fragment_shader(void *varyings_, void *uniforms_,
                           int *discard, int backface) {
    pbr_varyings_t *varyings = (pbr_varyings_t*)varyings_;
    pbr_uniforms_t *uniforms = (pbr_uniforms_t*)uniforms_;

    if (uniforms->shadow_pass) {
        return shadow_fragment_shader(varyings, uniforms, discard);
    } else {
        return common_fragment_shader(varyings, uniforms, discard, backface);
    }
}

/* high-level api */

static void update_model(model_t *model, perframe_t *perframe) {
    float ambient_intensity = perframe->ambient_intensity;
    float punctual_intensity = perframe->punctual_intensity;
    skeleton_t *skeleton = model->skeleton;
    mat4_t model_matrix = model->transform;
    mat3_t normal_matrix;
    mat4_t *joint_matrices;
    mat3_t *joint_n_matrices;
    pbr_uniforms_t *uniforms;

    if (skeleton) {
        skeleton_update_joints(skeleton, perframe->frame_time);
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
    uniforms->light_dir = perframe->light_dir;
    uniforms->camera_pos = perframe->camera_pos;
    uniforms->model_matrix = model_matrix;
    uniforms->normal_matrix = normal_matrix;
    uniforms->light_vp_matrix = mat4_mul_mat4(perframe->light_proj_matrix,
                                              perframe->light_view_matrix);
    uniforms->camera_vp_matrix = mat4_mul_mat4(perframe->camera_proj_matrix,
                                               perframe->camera_view_matrix);
    uniforms->joint_matrices = joint_matrices;
    uniforms->joint_n_matrices = joint_n_matrices;
    uniforms->ambient_intensity = float_clamp(ambient_intensity, 0, 5);
    uniforms->punctual_intensity = float_clamp(punctual_intensity, 0, 5);
    uniforms->shadow_map = perframe->shadow_map;
    uniforms->layer_view = perframe->layer_view;
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

static model_t *create_model(const char *mesh, mat4_t transform,
                             const char *skeleton, int attached,
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
    model->program = program;
    model->transform = transform;
    model->skeleton = cache_acquire_skeleton(skeleton);
    model->attached = attached;
    model->opaque = !enable_blend;
    model->distance = 0;
    model->update = update_model;
    model->draw = draw_model;
    model->release = release_model;

    return model;
}

static texture_t *acquire_color_texture(const char *filename) {
    return cache_acquire_texture(filename, USAGE_HDR_COLOR);
}

static texture_t *acquire_data_texture(const char *filename) {
    return cache_acquire_texture(filename, USAGE_LDR_DATA);
}

model_t *pbrm_create_model(const char *mesh, mat4_t transform,
                           const char *skeleton, int attached,
                           pbrm_material_t *material, const char *env_name) {
    pbr_uniforms_t *uniforms;
    model_t *model;

    model = create_model(mesh, transform, skeleton, attached,
                         material->double_sided, material->enable_blend);

    uniforms = (pbr_uniforms_t*)program_get_uniforms(model->program);
    uniforms->basecolor_factor = material->basecolor_factor;
    uniforms->metalness_factor = float_saturate(material->metalness_factor);
    uniforms->roughness_factor = float_saturate(material->roughness_factor);
    uniforms->basecolor_map = acquire_color_texture(material->basecolor_map);
    uniforms->metalness_map = acquire_data_texture(material->metalness_map);
    uniforms->roughness_map = acquire_data_texture(material->roughness_map);
    uniforms->normal_map = acquire_data_texture(material->normal_map);
    uniforms->occlusion_map = acquire_data_texture(material->occlusion_map);
    uniforms->emission_map = acquire_color_texture(material->emission_map);
    uniforms->ibldata = cache_acquire_ibldata(env_name);
    uniforms->alpha_cutoff = material->alpha_cutoff;
    uniforms->workflow = METALNESS_WORKFLOW;
    uniforms->layer_view = -1;

    return model;
}

model_t *pbrs_create_model(const char *mesh, mat4_t transform,
                           const char *skeleton, int attached,
                           pbrs_material_t *material, const char *env_name) {
    pbr_uniforms_t *uniforms;
    model_t *model;

    model = create_model(mesh, transform, skeleton, attached,
                         material->double_sided, material->enable_blend);

    uniforms = (pbr_uniforms_t*)program_get_uniforms(model->program);
    uniforms->diffuse_factor = material->diffuse_factor;
    uniforms->specular_factor = material->specular_factor;
    uniforms->glossiness_factor = float_saturate(material->glossiness_factor);
    uniforms->diffuse_map = acquire_color_texture(material->diffuse_map);
    uniforms->specular_map = acquire_color_texture(material->specular_map);
    uniforms->glossiness_map = acquire_data_texture(material->glossiness_map);
    uniforms->normal_map = acquire_data_texture(material->normal_map);
    uniforms->occlusion_map = acquire_data_texture(material->occlusion_map);
    uniforms->emission_map = acquire_color_texture(material->emission_map);
    uniforms->ibldata = cache_acquire_ibldata(env_name);
    uniforms->alpha_cutoff = material->alpha_cutoff;
    uniforms->workflow = SPECULAR_WORKFLOW;
    uniforms->layer_view = -1;

    return model;
}
