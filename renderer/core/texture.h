#ifndef TEXTURE_H
#define TEXTURE_H

#include "graphics.h"
#include "maths.h"

typedef enum {
    USAGE_LDR_COLOR,
    USAGE_LDR_DATA,
    USAGE_HDR_COLOR,
    USAGE_HDR_DATA
} usage_t;

typedef struct {
    int width, height;
    vec4_t *buffer;
} texture_t;

typedef struct {
    texture_t *faces[6];
} cubemap_t;

/* texture related functions */
texture_t *texture_create(int width, int height);
void texture_release(texture_t *texture);
texture_t *texture_from_file(const char *filename, usage_t usage);
void texture_from_colorbuffer(texture_t *texture, framebuffer_t *framebuffer);
void texture_from_depthbuffer(texture_t *texture, framebuffer_t *framebuffer);
vec4_t texture_repeat_sample(texture_t *texture, vec2_t texcoord);
vec4_t texture_clamp_sample(texture_t *texture, vec2_t texcoord);
vec4_t texture_sample(texture_t *texture, vec2_t texcoord);

/* cubemap related functions */
cubemap_t *cubemap_from_files(const char *positive_x, const char *negative_x,
                              const char *positive_y, const char *negative_y,
                              const char *positive_z, const char *negative_z,
                              usage_t usage);
void cubemap_release(cubemap_t *cubemap);
vec4_t cubemap_repeat_sample(cubemap_t *cubemap, vec3_t direction);
vec4_t cubemap_clamp_sample(cubemap_t *cubemap, vec3_t direction);
vec4_t cubemap_sample(cubemap_t *cubemap, vec3_t direction);

#endif
