#ifndef CACHE_HELPER_H
#define CACHE_HELPER_H

#include "../core/api.h"

struct ibldata;

/* mesh related functions */
mesh_t *cache_acquire_mesh(const char *filename);
void cache_release_mesh(mesh_t *mesh);

/* skeleton related functions */
skeleton_t *cache_acquire_skeleton(const char *filename);
void cache_release_skeleton(skeleton_t *skeleton);

/* texture related functions */
texture_t *cache_acquire_texture(const char *filename, usage_t usage);
void cache_release_texture(texture_t *texture);

/* skybox related functions */
cubemap_t *cache_acquire_skybox(const char *skybox_name, int blur_level);
void cache_release_skybox(cubemap_t *skybox);

/* ibldata related functions */
struct ibldata *cache_acquire_ibldata(const char *env_name);
void cache_release_ibldata(struct ibldata *ibldata);

/* misc cache functions */
void cache_cleanup(void);

#endif
