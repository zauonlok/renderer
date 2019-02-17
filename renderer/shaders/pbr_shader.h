#ifndef PBR_SHADER_H
#define PBR_SHADER_H

#include "../core/api.h"

typedef struct {
    int mip_level;
    cubemap_t *diffuse;
    cubemap_t *specular[15];
    texture_t *brdf_lut;
    /* for reference counting */
    const char *env_name;
    int ref_count;
} ibldata_t;

/* shading related functions */
vec3_t pbr_dir_shade(vec3_t light_dir, float roughness,
                     vec3_t normal_dir, vec3_t view_dir,
                     vec3_t diffuse_color, vec3_t specular_color);
vec3_t pbr_ibl_shade(ibldata_t *ibldata, float roughness,
                     vec3_t normal_dir, vec3_t view_dir,
                     vec3_t diffuse_color, vec3_t specular_color);
mat3_t pbr_build_tbn(vec3_t normal, mat3_t normal_matrix,
                     vec4_t tangent, mat4_t model_matrix);
vec3_t pbr_get_normal(mat3_t tbn_matrix, vec2_t texcoord,
                      texture_t *normal_texture);
vec3_t pbr_tone_map(vec3_t color);

/* ibldata related functions */
ibldata_t *pbr_acquire_ibldata(const char *env_name);
void pbr_release_ibldata(ibldata_t *ibldata);

#endif
