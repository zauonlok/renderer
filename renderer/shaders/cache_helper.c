#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "../core/api.h"
#include "cache_helper.h"

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

    cached_texture.filename = (char*)malloc(strlen(filename) + 1);
    strcpy(cached_texture.filename, filename);
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
