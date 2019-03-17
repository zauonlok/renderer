#ifndef SHADER_HELPER_H
#define SHADER_HELPER_H

#include "../core/api.h"

typedef struct ibldata ibldata_t;

/* pbr related functions */
vec3_t pbr_dir_shade(vec3_t light_dir, float roughness,
                     vec3_t normal_dir, vec3_t view_dir,
                     vec3_t diffuse_color, vec3_t specular_color);
vec3_t pbr_ibl_shade(ibldata_t *ibldata, float roughness,
                     vec3_t normal_dir, vec3_t view_dir,
                     vec3_t diffuse_color, vec3_t specular_color);
vec3_t pbr_tone_map(vec3_t color);

/* normal related functions */
mat3_t normal_build_tbn(vec3_t normal, mat3_t normal_matrix,
                        vec4_t tangent, mat4_t model_matrix);
vec3_t normal_from_texture(mat3_t tbn_matrix, vec2_t texcoord,
                           texture_t *normal_texture);

/* cache related functions */
mesh_t *cache_acquire_mesh(const char *filename);
void cache_release_mesh(mesh_t *mesh);
texture_t *cache_acquire_texture(const char *filename, int srgb2linear);
void cache_release_texture(texture_t *texture);
cubemap_t *cache_acquire_skybox(const char *skybox_name);
void cache_release_skybox(cubemap_t *skybox);
ibldata_t *cache_acquire_ibldata(const char *env_name);
void cache_release_ibldata(ibldata_t *ibldata);

#endif
