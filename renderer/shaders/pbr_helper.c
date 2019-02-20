#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/api.h"
#include "pbr_helper.h"

/* shading related functions */

static float max_float(float a, float b) {
    return a > b ? a : b;
}

static float get_distribution(float n_dot_h, float alpha2) {
    float n_dot_h_2 = n_dot_h * n_dot_h;
    float factor = n_dot_h_2 * (alpha2 - 1) + 1;
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

vec3_t pbr_dir_shade(vec3_t light_dir, float roughness,
                     vec3_t normal_dir, vec3_t view_dir,
                     vec3_t diffuse_color, vec3_t specular_color) {
    float n_dot_l = vec3_dot(normal_dir, light_dir);
    if (n_dot_l > 0) {
        vec3_t half_dir = vec3_normalize(vec3_add(light_dir, view_dir));
        float n_dot_v = max_float(vec3_dot(normal_dir, view_dir), 0);
        float n_dot_h = max_float(vec3_dot(normal_dir, half_dir), 0);
        float v_dot_h = max_float(vec3_dot(view_dir, half_dir), 0);

        float alpha = roughness * roughness;
        float alpha2 = alpha * alpha;

        float d_term = get_distribution(n_dot_h, alpha2);
        float v_term = get_visibility(n_dot_v, n_dot_l, alpha2);
        vec3_t f_term = get_fresnel(v_dot_h, specular_color);

        vec3_t diffuse_brdf = vec3_div(diffuse_color, PI);
        vec3_t specular_brdf = vec3_mul(f_term, v_term * d_term);

        float combined_x = (1 - f_term.x) * diffuse_brdf.x + specular_brdf.x;
        float combined_y = (1 - f_term.y) * diffuse_brdf.y + specular_brdf.y;
        float combined_z = (1 - f_term.z) * diffuse_brdf.z + specular_brdf.z;
        vec3_t combined_brdf = vec3_new(combined_x, combined_y, combined_z);

        return vec3_mul(combined_brdf, n_dot_l);
    } else {
        return vec3_new(0, 0, 0);
    }
}

static vec3_t get_incident_dir(vec3_t normal, vec3_t view_dir) {
    return vec3_sub(vec3_mul(normal, 2 * vec3_dot(view_dir, normal)), view_dir);
}

vec3_t pbr_ibl_shade(ibldata_t *ibldata, float roughness,
                     vec3_t normal_dir, vec3_t view_dir,
                     vec3_t diffuse_color, vec3_t specular_color) {
    cubemap_t *diffuse_envmap = ibldata->diffuse;
    vec4_t diffuse_sample = cubemap_sample(diffuse_envmap, normal_dir);
    vec3_t diffuse_light = vec3_from_vec4(diffuse_sample);
    vec3_t diffuse_shade = vec3_modulate(diffuse_light, diffuse_color);

    float n_dot_v = max_float(vec3_dot(normal_dir, view_dir), 0);
    vec2_t brdf_texcoord = vec2_new(n_dot_v, roughness);
    vec4_t brdf_sample = texture_sample(ibldata->brdf_lut, brdf_texcoord);
    float specular_scale = brdf_sample.x;
    float specular_bias = brdf_sample.y;

    vec3_t incident_dir = get_incident_dir(normal_dir, view_dir);
    int specular_lod = (int)(roughness * (ibldata->mip_level - 1));
    cubemap_t *specular_envmap = ibldata->specular[specular_lod];
    vec4_t specular_sample = cubemap_sample(specular_envmap, incident_dir);

    float specular_x = specular_sample.x * specular_scale + specular_bias;
    float specular_y = specular_sample.y * specular_scale + specular_bias;
    float specular_z = specular_sample.z * specular_scale + specular_bias;
    vec3_t specular_light = vec3_new(specular_x, specular_y, specular_z);
    vec3_t specular_shade = vec3_modulate(specular_light, specular_color);

    return vec3_add(diffuse_shade, specular_shade);
}

mat3_t pbr_build_tbn(vec3_t normal, mat3_t normal_matrix,
                     vec4_t tangent, mat4_t model_matrix) {
    vec3_t world_normal = vec3_normalize(mat3_mul_vec3(normal_matrix, normal));

    mat3_t model_matrix_ = mat3_from_mat4(model_matrix);
    vec3_t local_tangent = vec3_from_vec4(tangent);
    vec3_t world_tangent_ = mat3_mul_vec3(model_matrix_, local_tangent);
    vec3_t world_tangent = vec3_normalize(world_tangent_);

    float handedness = tangent.w;
    vec3_t world_bitangent_ = vec3_cross(world_normal, world_tangent);
    vec3_t world_bitangent = vec3_mul(world_bitangent_, handedness);

    return mat3_from_cols(world_tangent, world_bitangent, world_normal);
}

vec3_t pbr_get_normal(mat3_t tbn_matrix, vec2_t texcoord,
                      texture_t *normal_texture) {
    if (normal_texture) {
        vec4_t sample = texture_sample(normal_texture, texcoord);
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

vec3_t pbr_tone_map(vec3_t color) {
    color.x = (float)pow(color.x, 1 / 2.2);
    color.y = (float)pow(color.y, 1 / 2.2);
    color.z = (float)pow(color.z, 1 / 2.2);
    return color;
}

/* ibldata related functions */

typedef struct {
    const char *env_name;
    int mip_level;
} envinfo_t;

static envinfo_t g_envinfo[] = {
    {"papermill", 10},
};

static ibldata_t *g_ibldata[ARRAY_SIZE(g_envinfo)];

static ibldata_t *load_ibldata(const char *env_name, int mip_level) {
    const char *faces[6] = {"right", "left", "top", "bottom", "front", "back"};
    const char *format = "assets/common/%s/%s_%s_%d.tga";
    char paths[6][128];
    ibldata_t *ibldata;
    int i, j;

    ibldata = (ibldata_t*)malloc(sizeof(ibldata_t));
    memset(ibldata, 0, sizeof(ibldata_t));
    ibldata->mip_level = mip_level;
    ibldata->env_name = env_name;
    ibldata->ref_count = 1;

    /* load a diffuse envmap */
    for (j = 0; j < 6; j++) {
        sprintf(paths[j], format, env_name, "diffuse", faces[j], 0);
    }
    ibldata->diffuse = cubemap_from_files(paths[0], paths[1], paths[2],
                                          paths[3], paths[4], paths[5]);
    cubemap_srgb2linear(ibldata->diffuse);

    /* load specular envmaps */
    for (i = 0; i < mip_level; i++) {
        for (j = 0; j < 6; j++) {
            sprintf(paths[j], format, env_name, "specular", faces[j], i);
        }
        ibldata->specular[i] = cubemap_from_files(paths[0], paths[1], paths[2],
                                                  paths[3], paths[4], paths[5]);
        cubemap_srgb2linear(ibldata->specular[i]);
    }

    /* load brdf lookup table */
    ibldata->brdf_lut = texture_from_file("assets/common/brdf_lut.tga");

    return ibldata;
}

static void free_ibldata(ibldata_t *ibldata) {
    int i;
    cubemap_release(ibldata->diffuse);
    for (i = 0; i < ibldata->mip_level; i++) {
        cubemap_release(ibldata->specular[i]);
    }
    texture_release(ibldata->brdf_lut);
    free(ibldata);
}

ibldata_t *pbr_acquire_ibldata(const char *env_name) {
    int num_envinfo = ARRAY_SIZE(g_envinfo);
    int i;
    for (i = 0; i < num_envinfo; i++) {
        if (strcmp(env_name, g_envinfo[i].env_name) == 0) {
            ibldata_t *ibldata;
            if (g_ibldata[i] == NULL) {
                ibldata = load_ibldata(env_name, g_envinfo[i].mip_level);
                g_ibldata[i] = ibldata;
            } else {
                ibldata = g_ibldata[i];
                ibldata->ref_count += 1;
            }
            return ibldata;
        }
    }
    assert(0);
    return NULL;
}

void pbr_release_ibldata(ibldata_t *ibldata) {
    ibldata->ref_count -= 1;
    if (ibldata->ref_count <= 0) {
        int num_envinfo = ARRAY_SIZE(g_envinfo);
        int i;
        for (i = 0; i < num_envinfo; i++) {
            if (strcmp(ibldata->env_name, g_envinfo[i].env_name) == 0) {
                assert(g_ibldata[i] == ibldata);
                free_ibldata(ibldata);
                g_ibldata[i] = NULL;
                return;
            }
        }
        assert(0);
    }
}
