#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "graphics.h"
#include "image.h"
#include "maths.h"
#include "texture.h"

/* texture related functions */

texture_t *texture_create(int width, int height) {
    int buffer_size = sizeof(vec4_t) * width * height;
    texture_t *texture;

    assert(width > 0 && height > 0);

    texture = (texture_t*)malloc(sizeof(texture_t));
    texture->width = width;
    texture->height = height;
    texture->buffer = (vec4_t*)malloc(buffer_size);
    memset(texture->buffer, 0, buffer_size);

    return texture;
}

void texture_release(texture_t *texture) {
    free(texture->buffer);
    free(texture);
}

static void ldr_image_to_texture(image_t *image, texture_t *texture) {
    int num_pixels = image->width * image->height;
    int i;

    for (i = 0; i < num_pixels; i++) {
        unsigned char *pixel = &image->ldr_buffer[i * image->channels];
        vec4_t texel = {0, 0, 0, 1};
        if (image->channels == 1) {             /* GL_LUMINANCE */
            texel.x = texel.y = texel.z = float_from_uchar(pixel[0]);
        } else if (image->channels == 2) {      /* GL_LUMINANCE_ALPHA */
            texel.x = texel.y = texel.z = float_from_uchar(pixel[0]);
            texel.w = float_from_uchar(pixel[1]);
        } else if (image->channels == 3) {      /* GL_RGB */
            texel.x = float_from_uchar(pixel[0]);
            texel.y = float_from_uchar(pixel[1]);
            texel.z = float_from_uchar(pixel[2]);
        } else {                                /* GL_RGBA */
            texel.x = float_from_uchar(pixel[0]);
            texel.y = float_from_uchar(pixel[1]);
            texel.z = float_from_uchar(pixel[2]);
            texel.w = float_from_uchar(pixel[3]);
        }
        texture->buffer[i] = texel;
    }
}

static void hdr_image_to_texture(image_t *image, texture_t *texture) {
    int num_pixels = image->width * image->height;
    int i;

    for (i = 0; i < num_pixels; i++) {
        float *pixel = &image->hdr_buffer[i * image->channels];
        vec4_t texel = {0, 0, 0, 1};
        if (image->channels == 1) {             /* GL_LUMINANCE */
            texel.x = texel.y = texel.z = pixel[0];
        } else if (image->channels == 2) {      /* GL_LUMINANCE_ALPHA */
            texel.x = texel.y = texel.z = pixel[0];
            texel.w = pixel[1];
        } else if (image->channels == 3) {      /* GL_RGB */
            texel.x = pixel[0];
            texel.y = pixel[1];
            texel.z = pixel[2];
        } else {                                /* GL_RGBA */
            texel.x = pixel[0];
            texel.y = pixel[1];
            texel.z = pixel[2];
            texel.w = pixel[3];
        }
        texture->buffer[i] = texel;
    }
}

static void srgb_to_linear(texture_t *texture) {
    int num_pixels = texture->width * texture->height;
    int i;

    for (i = 0; i < num_pixels; i++) {
        vec4_t *pixel = &texture->buffer[i];
        pixel->x = float_srgb2linear(pixel->x);
        pixel->y = float_srgb2linear(pixel->y);
        pixel->z = float_srgb2linear(pixel->z);
    }
}

static void linear_to_srgb(texture_t *texture) {
    int num_pixels = texture->width * texture->height;
    int i;

    for (i = 0; i < num_pixels; i++) {
        vec4_t *pixel = &texture->buffer[i];
        pixel->x = float_linear2srgb(float_aces(pixel->x));
        pixel->y = float_linear2srgb(float_aces(pixel->y));
        pixel->z = float_linear2srgb(float_aces(pixel->z));
    }
}

texture_t *texture_from_file(const char *filename, usage_t usage) {
    texture_t *texture;
    image_t *image;

    image = image_load(filename);
    texture = texture_create(image->width, image->height);
    if (image->format == FORMAT_LDR) {
        ldr_image_to_texture(image, texture);
        if (usage == USAGE_HDR_COLOR) {
            srgb_to_linear(texture);
        }
    } else {
        hdr_image_to_texture(image, texture);
        if (usage == USAGE_LDR_COLOR) {
            linear_to_srgb(texture);
        }
    }
    image_release(image);

    return texture;
}

void texture_from_colorbuffer(texture_t *texture, framebuffer_t *framebuffer) {
    int num_pixels = texture->width * texture->height;
    int i;

    assert(texture->width == framebuffer->width);
    assert(texture->height == framebuffer->height);

    for (i = 0; i < num_pixels; i++) {
        unsigned char *color = &framebuffer->color_buffer[i * 4];
        float r = float_from_uchar(color[0]);
        float g = float_from_uchar(color[1]);
        float b = float_from_uchar(color[2]);
        float a = float_from_uchar(color[3]);
        texture->buffer[i] = vec4_new(r, g, b, a);
    }
}

void texture_from_depthbuffer(texture_t *texture, framebuffer_t *framebuffer) {
    int num_pixels = texture->width * texture->height;
    int i;

    assert(texture->width == framebuffer->width);
    assert(texture->height == framebuffer->height);

    for (i = 0; i < num_pixels; i++) {
        float depth = framebuffer->depth_buffer[i];
        texture->buffer[i] = vec4_new(depth, depth, depth, 1);
    }
}

vec4_t texture_repeat_sample(texture_t *texture, vec2_t texcoord) {
    float u = texcoord.x - (float)floor(texcoord.x);
    float v = texcoord.y - (float)floor(texcoord.y);
    int c = (int)((texture->width - 1) * u);
    int r = (int)((texture->height - 1) * v);
    int index = r * texture->width + c;
    return texture->buffer[index];
}

vec4_t texture_clamp_sample(texture_t *texture, vec2_t texcoord) {
    float u = float_saturate(texcoord.x);
    float v = float_saturate(texcoord.y);
    int c = (int)((texture->width - 1) * u);
    int r = (int)((texture->height - 1) * v);
    int index = r * texture->width + c;
    return texture->buffer[index];
}

vec4_t texture_sample(texture_t *texture, vec2_t texcoord) {
    return texture_repeat_sample(texture, texcoord);
}

/* cubemap related functions */

cubemap_t *cubemap_from_files(const char *positive_x, const char *negative_x,
                              const char *positive_y, const char *negative_y,
                              const char *positive_z, const char *negative_z,
                              usage_t usage) {
    cubemap_t *cubemap = (cubemap_t*)malloc(sizeof(cubemap_t));
    cubemap->faces[0] = texture_from_file(positive_x, usage);
    cubemap->faces[1] = texture_from_file(negative_x, usage);
    cubemap->faces[2] = texture_from_file(positive_y, usage);
    cubemap->faces[3] = texture_from_file(negative_y, usage);
    cubemap->faces[4] = texture_from_file(positive_z, usage);
    cubemap->faces[5] = texture_from_file(negative_z, usage);
    return cubemap;
}

void cubemap_release(cubemap_t *cubemap) {
    int i;
    for (i = 0; i < 6; i++) {
        texture_release(cubemap->faces[i]);
    }
    free(cubemap);
}

/*
 * for cubemap sampling, see subsection 3.7.5 of
 * https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
 */
static int select_cubemap_face(vec3_t direction, vec2_t *texcoord) {
    float abs_x = (float)fabs(direction.x);
    float abs_y = (float)fabs(direction.y);
    float abs_z = (float)fabs(direction.z);
    float ma, sc, tc;
    int face_index;

    if (abs_x > abs_y && abs_x > abs_z) {   /* major axis -> x */
        ma = abs_x;
        if (direction.x > 0) {                  /* positive x */
            face_index = 0;
            sc = -direction.z;
            tc = -direction.y;
        } else {                                /* negative x */
            face_index = 1;
            sc = +direction.z;
            tc = -direction.y;
        }
    } else if (abs_y > abs_z) {             /* major axis -> y */
        ma = abs_y;
        if (direction.y > 0) {                  /* positive y */
            face_index = 2;
            sc = +direction.x;
            tc = +direction.z;
        } else {                                /* negative y */
            face_index = 3;
            sc = +direction.x;
            tc = -direction.z;
        }
    } else {                                /* major axis -> z */
        ma = abs_z;
        if (direction.z > 0) {                  /* positive z */
            face_index = 4;
            sc = +direction.x;
            tc = -direction.y;
        } else {                                /* negative z */
            face_index = 5;
            sc = -direction.x;
            tc = -direction.y;
        }
    }

    texcoord->x = (sc / ma + 1) / 2;
    texcoord->y = (tc / ma + 1) / 2;
    return face_index;
}

vec4_t cubemap_repeat_sample(cubemap_t *cubemap, vec3_t direction) {
    vec2_t texcoord;
    int face_index = select_cubemap_face(direction, &texcoord);
    texcoord.y = 1 - texcoord.y;
    return texture_repeat_sample(cubemap->faces[face_index], texcoord);
}

vec4_t cubemap_clamp_sample(cubemap_t *cubemap, vec3_t direction) {
    vec2_t texcoord;
    int face_index = select_cubemap_face(direction, &texcoord);
    texcoord.y = 1 - texcoord.y;
    return texture_clamp_sample(cubemap->faces[face_index], texcoord);
}

vec4_t cubemap_sample(cubemap_t *cubemap, vec3_t direction) {
    return cubemap_repeat_sample(cubemap, direction);
}
