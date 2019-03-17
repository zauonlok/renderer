#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/api.h"
#include "shader_helper.h"

/* pbr related functions */

struct ibldata {
    int mip_level;
    cubemap_t *diffuse;
    cubemap_t *specular[15];
    texture_t *brdf_lut;
};

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

        /*
         * assume that light_color = {1, 1, 1} and light_intensity = 1,
         * then light_color * light_intensity * n_dot_l = n_dot_l
         */
        return vec3_mul(combined_brdf, n_dot_l);
    } else {
        return vec3_new(0, 0, 0);
    }
}

static vec3_t get_incident_dir(vec3_t normal_dir, vec3_t view_dir) {
    float n_dot_v = vec3_dot(normal_dir, view_dir);
    return vec3_sub(vec3_mul(normal_dir, 2 * n_dot_v), view_dir);
}

vec3_t pbr_ibl_shade(ibldata_t *ibldata, float roughness,
                     vec3_t normal_dir, vec3_t view_dir,
                     vec3_t diffuse_color, vec3_t specular_color_) {
    cubemap_t *diffuse_envmap = ibldata->diffuse;
    vec4_t diffuse_sample = cubemap_sample(diffuse_envmap, normal_dir);
    vec3_t diffuse_light = vec3_from_vec4(diffuse_sample);
    vec3_t diffuse_shade = vec3_modulate(diffuse_light, diffuse_color);

    float n_dot_v = float_max(vec3_dot(normal_dir, view_dir), 0);
    vec2_t brdf_texcoord = vec2_new(n_dot_v, roughness);
    vec4_t brdf_sample = texture_sample(ibldata->brdf_lut, brdf_texcoord);
    float specular_scale = brdf_sample.x;
    float specular_bias = brdf_sample.y;

    float specular_r = specular_color_.x * specular_scale + specular_bias;
    float specular_g = specular_color_.y * specular_scale + specular_bias;
    float specular_b = specular_color_.z * specular_scale + specular_bias;
    vec3_t specular_color = vec3_new(specular_r, specular_g, specular_b);

    vec3_t incident_dir = get_incident_dir(normal_dir, view_dir);
    int specular_lod = (int)(roughness * (ibldata->mip_level - 1));
    cubemap_t *specular_envmap = ibldata->specular[specular_lod];
    vec4_t specular_sample = cubemap_sample(specular_envmap, incident_dir);
    vec3_t specular_light = vec3_from_vec4(specular_sample);
    vec3_t specular_shade = vec3_modulate(specular_light, specular_color);

    return vec3_add(diffuse_shade, specular_shade);
}

vec3_t pbr_tone_map(vec3_t color) {
    float r = (float)pow(color.x, 1 / 2.2);
    float g = (float)pow(color.y, 1 / 2.2);
    float b = (float)pow(color.z, 1 / 2.2);
    return vec3_new(r, g, b);
}

/* normal related functions */

mat3_t normal_build_tbn(vec3_t normal, mat3_t normal_matrix,
                        vec4_t tangent, mat4_t model_matrix_) {
    vec3_t world_normal_ = mat3_mul_vec3(normal_matrix, normal);
    vec3_t world_normal = vec3_normalize(world_normal_);

    mat3_t model_matrix = mat3_from_mat4(model_matrix_);
    vec3_t local_tangent = vec3_from_vec4(tangent);
    vec3_t world_tangent_ = mat3_mul_vec3(model_matrix, local_tangent);
    vec3_t world_tangent = vec3_normalize(world_tangent_);

    float handedness = tangent.w;
    vec3_t world_bitangent_ = vec3_cross(world_normal, world_tangent);
    vec3_t world_bitangent = vec3_mul(world_bitangent_, handedness);

    return mat3_from_cols(world_tangent, world_bitangent, world_normal);
}

vec3_t normal_from_texture(mat3_t tbn_matrix, vec2_t texcoord,
                           texture_t *normal_texture) {
    vec4_t sample = texture_sample(normal_texture, texcoord);
    float normal_x = sample.x * 2 - 1;
    float normal_y = sample.y * 2 - 1;
    float normal_z = sample.z * 2 - 1;
    vec3_t normal = vec3_new(normal_x, normal_y, normal_z);
    return vec3_normalize(mat3_mul_vec3(tbn_matrix, normal));
}

/* cache (mesh) related functions */

typedef struct {
    char *filename;
    mesh_t *mesh;
    int references;
} cached_mesh_t;

static cached_mesh_t *g_meshes = NULL;

static char *duplicate_string(const char *source) {
    char *target = (char*)malloc(strlen(source) + 1);
    strcpy(target, source);
    return target;
}

mesh_t *cache_acquire_mesh(const char *filename) {
    cached_mesh_t cached_mesh;
    int num_meshes = darray_size(g_meshes);
    int i;

    for (i = 0; i < num_meshes; i++) {
        if (strcmp(g_meshes[i].filename, filename) == 0) {
            if (g_meshes[i].references > 0) {
                g_meshes[i].references += 1;
            } else {
                assert(g_meshes[i].references == 0);
                assert(g_meshes[i].mesh == NULL);
                g_meshes[i].mesh = mesh_load(filename);
                g_meshes[i].references = 1;
            }
            return g_meshes[i].mesh;
        }
    }

    cached_mesh.filename = duplicate_string(filename);
    cached_mesh.mesh = mesh_load(filename);
    cached_mesh.references = 1;
    darray_push(g_meshes, cached_mesh);
    return cached_mesh.mesh;
}

void cache_release_mesh(mesh_t *mesh) {
    int num_meshes = darray_size(g_meshes);
    int i;
    for (i = 0; i < num_meshes; i++) {
        if (g_meshes[i].mesh == mesh) {
            assert(g_meshes[i].references > 0);
            g_meshes[i].references -= 1;
            if (g_meshes[i].references == 0) {
                mesh_release(g_meshes[i].mesh);
                g_meshes[i].mesh = NULL;
            }
            return;
        }
    }
    assert(0);
}

/* cache (texture) related functions */

typedef struct {
    char *filename;
    int srgb2linear;
    texture_t *texture;
    int references;
} cached_texture_t;

static cached_texture_t *g_textures = NULL;

texture_t *cache_acquire_texture(const char *filename, int srgb2linear) {
    cached_texture_t cached_texture;
    int num_textures = darray_size(g_textures);
    int i;

    assert(srgb2linear == 0 || srgb2linear == 1);

    if (filename == NULL) {
        return NULL;
    }

    for (i = 0; i < num_textures; i++) {
        if (strcmp(g_textures[i].filename, filename) == 0) {
            if (g_textures[i].srgb2linear == srgb2linear) {
                if (g_textures[i].references > 0) {
                    g_textures[i].references += 1;
                } else {
                    assert(g_textures[i].references == 0);
                    assert(g_textures[i].texture == NULL);
                    g_textures[i].texture = texture_from_file(filename);
                    if (srgb2linear) {
                        texture_srgb2linear(g_textures[i].texture);
                    }
                    g_textures[i].references = 1;
                }
                return g_textures[i].texture;
            }
        }
    }

    cached_texture.filename = duplicate_string(filename);
    cached_texture.srgb2linear = srgb2linear;
    cached_texture.texture = texture_from_file(filename);
    if (srgb2linear) {
        texture_srgb2linear(cached_texture.texture);
    }
    cached_texture.references = 1;
    darray_push(g_textures, cached_texture);
    return cached_texture.texture;
}

void cache_release_texture(texture_t *texture) {
    int num_textures = darray_size(g_textures);
    int i;

    if (texture == NULL) {
        return;
    }

    for (i = 0; i < num_textures; i++) {
        if (g_textures[i].texture == texture) {
            assert(g_textures[i].references > 0);
            g_textures[i].references -= 1;
            if (g_textures[i].references == 0) {
                texture_release(g_textures[i].texture);
                g_textures[i].texture = NULL;
            }
            return;
        }
    }
    assert(0);
}

/* cache (skybox) related functions */

typedef struct {
    const char *skybox_name;
    cubemap_t *skybox;
    int references;
} skybox_t;

static skybox_t g_skyboxes[] = {
    {"papermill", NULL, 0},
};

static cubemap_t *load_skybox(const char *skybox_name) {
    const char *faces[6] = {"right", "left", "top", "bottom", "front", "back"};
    const char *format = "assets/common/%s/skybox_%s.tga";
    char paths[6][128];
    cubemap_t *skybox;
    int i;

    for (i = 0; i < 6; i++) {
        sprintf(paths[i], format, skybox_name, faces[i]);
    }
    skybox = cubemap_from_files(paths[0], paths[1], paths[2],
                                paths[3], paths[4], paths[5]);

    return skybox;
}

static void free_skybox(cubemap_t *skybox) {
    cubemap_release(skybox);
}

cubemap_t *cache_acquire_skybox(const char *skybox_name) {
    int num_skyboxes = ARRAY_SIZE(g_skyboxes);
    int i;
    for (i = 0; i < num_skyboxes; i++) {
        if (strcmp(g_skyboxes[i].skybox_name, skybox_name) == 0) {
            if (g_skyboxes[i].references > 0) {
                g_skyboxes[i].references += 1;
            } else {
                assert(g_skyboxes[i].skybox == NULL);
                assert(g_skyboxes[i].references == 0);
                g_skyboxes[i].skybox = load_skybox(skybox_name);
                g_skyboxes[i].references = 1;
            }
            return g_skyboxes[i].skybox;
        }
    }
    assert(0);
    return NULL;
}

void cache_release_skybox(cubemap_t *skybox) {
    int num_skyboxes = ARRAY_SIZE(g_skyboxes);
    int i;
    for (i = 0; i < num_skyboxes; i++) {
        if (g_skyboxes[i].skybox == skybox) {
            assert(g_skyboxes[i].references > 0);
            g_skyboxes[i].references -= 1;
            if (g_skyboxes[i].references == 0) {
                free_skybox(g_skyboxes[i].skybox);
                g_skyboxes[i].skybox = NULL;
            }
            return;
        }
    }
    assert(0);
}

/* cache (ibldata) related functions */

typedef struct {
    const char *env_name;
    int mip_level;
    ibldata_t *ibldata;
    int references;
} envinfo_t;

static envinfo_t g_envinfo[] = {
    {"papermill", 10, NULL, 0},
};

static ibldata_t *load_ibldata(const char *env_name, int mip_level) {
    const char *faces[6] = {"right", "left", "top", "bottom", "front", "back"};
    const char *format = "assets/common/%s/%s_%s_%d.tga";
    char paths[6][128];
    ibldata_t *ibldata;
    int i, j;

    ibldata = (ibldata_t*)malloc(sizeof(ibldata_t));
    memset(ibldata, 0, sizeof(ibldata_t));
    ibldata->mip_level = mip_level;

    /* diffuse environment map */
    for (j = 0; j < 6; j++) {
        sprintf(paths[j], format, env_name, "diffuse", faces[j], 0);
    }
    ibldata->diffuse = cubemap_from_files(paths[0], paths[1], paths[2],
                                          paths[3], paths[4], paths[5]);
    cubemap_srgb2linear(ibldata->diffuse);

    /* specular environment maps */
    for (i = 0; i < mip_level; i++) {
        for (j = 0; j < 6; j++) {
            sprintf(paths[j], format, env_name, "specular", faces[j], i);
        }
        ibldata->specular[i] = cubemap_from_files(paths[0], paths[1], paths[2],
                                                  paths[3], paths[4], paths[5]);
        cubemap_srgb2linear(ibldata->specular[i]);
    }

    /* brdf lookup table */
    ibldata->brdf_lut = cache_acquire_texture("assets/common/brdf_lut.tga", 0);

    return ibldata;
}

static void free_ibldata(ibldata_t *ibldata) {
    int i;
    cubemap_release(ibldata->diffuse);
    for (i = 0; i < ibldata->mip_level; i++) {
        cubemap_release(ibldata->specular[i]);
    }
    cache_release_texture(ibldata->brdf_lut);
    free(ibldata);
}

ibldata_t *cache_acquire_ibldata(const char *env_name) {
    int num_envinfo = ARRAY_SIZE(g_envinfo);
    int i;
    for (i = 0; i < num_envinfo; i++) {
        if (strcmp(g_envinfo[i].env_name, env_name) == 0) {
            if (g_envinfo[i].references > 0) {
                g_envinfo[i].references += 1;
            } else {
                int mip_level = g_envinfo[i].mip_level;
                assert(g_envinfo[i].ibldata == NULL);
                assert(g_envinfo[i].references == 0);
                g_envinfo[i].ibldata = load_ibldata(env_name, mip_level);
                g_envinfo[i].references = 1;
            }
            return g_envinfo[i].ibldata;
        }
    }
    assert(0);
    return NULL;
}

void cache_release_ibldata(ibldata_t *ibldata) {
    int num_envinfo = ARRAY_SIZE(g_envinfo);
    int i;
    for (i = 0; i < num_envinfo; i++) {
        if (g_envinfo[i].ibldata == ibldata) {
            assert(g_envinfo[i].references > 0);
            g_envinfo[i].references -= 1;
            if (g_envinfo[i].references == 0) {
                free_ibldata(g_envinfo[i].ibldata);
                g_envinfo[i].ibldata = NULL;
            }
            return;
        }
    }
    assert(0);
}
