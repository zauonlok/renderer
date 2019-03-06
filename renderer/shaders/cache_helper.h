#ifndef CACHE_HELPER_H
#define CACHE_HELPER_H

#include "../core/api.h"

texture_t *cache_acquire_texture(const char *filename, int srgb2linear);
void cache_release_texture(texture_t *texture);

#endif
