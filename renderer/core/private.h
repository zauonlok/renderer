#ifndef PRIVATE_H
#define PRIVATE_H

#include "graphics.h"
#include "image.h"
#include "texture.h"

/* image formats */
image_t *private_load_tga_image(const char *filename);
void private_save_tga_image(image_t *image, const char *filename);
texture_t *private_load_hdr_image(const char *filename);
void private_save_hdr_image(texture_t *texture, const char *filename);

/* image blitting */
void private_blit_image_bgr(image_t *source, image_t *target);
void private_blit_image_rgb(image_t *source, image_t *target);
void private_blit_buffer_bgr(framebuffer_t *source, image_t *target);
void private_blit_buffer_rgb(framebuffer_t *source, image_t *target);

/* misc functions */
const char *private_get_extension(const char *filename);
unsigned char *private_get_pixel(image_t *image, int row, int col);

#endif
