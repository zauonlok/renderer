#ifndef TEXTURE_H
#define TEXTURE_H

#include "geometry.h"
#include "graphics.h"
#include "image.h"

typedef struct texture texture_t;
typedef struct cubemap cubemap_t;

/* texture related functions */
texture_t *texture_from_file(const char *filename);
texture_t *texture_from_image(image_t *image);
texture_t *texture_from_depth(framebuffer_t *framebuffer, texture_t *existing);
void texture_release(texture_t *texture);
void texture_srgb2linear(texture_t *texture);
vec4_t texture_sample(texture_t *texture, vec2_t texcoord);

/* cubemap related functions */
cubemap_t *cubemap_from_files(const char *positive_x, const char *negative_x,
                              const char *positive_y, const char *negative_y,
                              const char *positive_z, const char *negative_z);
void cubemap_release(cubemap_t *cubemap);
void cubemap_srgb2linear(cubemap_t *cubemap);
vec4_t cubemap_sample(cubemap_t *cubemap, vec3_t direction);

#endif
