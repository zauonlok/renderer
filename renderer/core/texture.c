#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "graphics.h"
#include "image.h"
#include "maths.h"
#include "texture.h"

/* texture related functions */

texture_t *texture_create(int width, int height, format_t format) {
    int num_elems = width * height * 4;
    texture_t *texture;

    assert(width > 0 && height > 0);
    assert(format == FORMAT_LDR || format == FORMAT_HDR);

    texture = (texture_t*)malloc(sizeof(texture_t));
    texture->format = format;
    texture->width = width;
    texture->height = height;
    texture->ldr_buffer = NULL;
    texture->hdr_buffer = NULL;

    if (format == FORMAT_LDR) {
        int size = sizeof(unsigned char) * num_elems;
        texture->ldr_buffer = (unsigned char*)malloc(size);
        memset(texture->ldr_buffer, 0, size);
    } else {
        int size = sizeof(float) * num_elems;
        texture->hdr_buffer = (float*)malloc(size);
        memset(texture->hdr_buffer, 0, size);
    }

    return texture;
}

void texture_release(texture_t *texture) {
    free(texture->ldr_buffer);
    free(texture->hdr_buffer);
    free(texture);
}

static void ldr_image_to_texture(image_t *image, texture_t *texture) {
    unsigned char *img_buffer = image->ldr_buffer;
    unsigned char *tex_buffer = texture->ldr_buffer;
    int r, c;

    for (r = 0; r < image->height; r++) {
        for (c = 0; c < image->width; c++) {
            int img_index = (r * image->width + c) * image->channels;
            int tex_index = (r * image->width + c) * 4;
            if (image->channels == 1) {             /* GL_LUMINANCE */
                tex_buffer[tex_index + 0] = img_buffer[img_index + 0];
                tex_buffer[tex_index + 1] = img_buffer[img_index + 0];
                tex_buffer[tex_index + 2] = img_buffer[img_index + 0];
                tex_buffer[tex_index + 3] = 1;
            } else if (image->channels == 2) {      /* GL_LUMINANCE_ALPHA */
                tex_buffer[tex_index + 0] = img_buffer[img_index + 0];
                tex_buffer[tex_index + 1] = img_buffer[img_index + 0];
                tex_buffer[tex_index + 2] = img_buffer[img_index + 0];
                tex_buffer[tex_index + 3] = img_buffer[img_index + 1];
            } else if (image->channels == 3) {      /* GL_RGB */
                tex_buffer[tex_index + 0] = img_buffer[img_index + 0];
                tex_buffer[tex_index + 1] = img_buffer[img_index + 1];
                tex_buffer[tex_index + 2] = img_buffer[img_index + 2];
                tex_buffer[tex_index + 3] = 1;
            } else {                                /* GL_RGBA */
                tex_buffer[tex_index + 0] = img_buffer[img_index + 0];
                tex_buffer[tex_index + 1] = img_buffer[img_index + 1];
                tex_buffer[tex_index + 2] = img_buffer[img_index + 2];
                tex_buffer[tex_index + 3] = img_buffer[img_index + 3];
            }
        }
    }
}

static void hdr_image_to_texture(image_t *image, texture_t *texture) {
    float *img_buffer = image->hdr_buffer;
    float *tex_buffer = texture->hdr_buffer;
    int r, c;

    for (r = 0; r < image->height; r++) {
        for (c = 0; c < image->width; c++) {
            int img_index = (r * image->width + c) * image->channels;
            int tex_index = (r * image->width + c) * 4;
            if (image->channels == 1) {             /* GL_LUMINANCE */
                tex_buffer[tex_index + 0] = img_buffer[img_index + 0];
                tex_buffer[tex_index + 1] = img_buffer[img_index + 0];
                tex_buffer[tex_index + 2] = img_buffer[img_index + 0];
                tex_buffer[tex_index + 3] = 1;
            } else if (image->channels == 2) {      /* GL_LUMINANCE_ALPHA */
                tex_buffer[tex_index + 0] = img_buffer[img_index + 0];
                tex_buffer[tex_index + 1] = img_buffer[img_index + 0];
                tex_buffer[tex_index + 2] = img_buffer[img_index + 0];
                tex_buffer[tex_index + 3] = img_buffer[img_index + 1];
            } else if (image->channels == 3) {      /* GL_RGB */
                tex_buffer[tex_index + 0] = img_buffer[img_index + 0];
                tex_buffer[tex_index + 1] = img_buffer[img_index + 1];
                tex_buffer[tex_index + 2] = img_buffer[img_index + 2];
                tex_buffer[tex_index + 3] = 1;
            } else {                                /* GL_RGBA */
                tex_buffer[tex_index + 0] = img_buffer[img_index + 0];
                tex_buffer[tex_index + 1] = img_buffer[img_index + 1];
                tex_buffer[tex_index + 2] = img_buffer[img_index + 2];
                tex_buffer[tex_index + 3] = img_buffer[img_index + 3];
            }
        }
    }
}

texture_t *texture_from_file(const char *filename, usage_t usage) {
    texture_t *texture;
    image_t *image;

    image = image_load(filename);
    if (image->format == FORMAT_LDR) {
        if (usage == USAGE_HDR_COLOR) {
            image_ldr2hdr(image);
            image_srgb2linear(image);
        }
        if (usage == USAGE_HDR_DATA) {
            image_ldr2hdr(image);
        }
    } else {
        if (usage == USAGE_LDR_COLOR) {
            image_linear2srgb(image);
            image_hdr2ldr(image);
        }
        if (usage == USAGE_LDR_DATA) {
            image_hdr2ldr(image);
        }
    }
    texture = texture_create(image->width, image->height, image->format);
    if (image->format == FORMAT_LDR) {
        ldr_image_to_texture(image, texture);
    } else {
        hdr_image_to_texture(image, texture);
    }
    image_release(image);

    return texture;
}

void texture_from_colorbuffer(texture_t *texture, framebuffer_t *framebuffer) {
    int num_pixels = texture->width * texture->height;
    int i;

    assert(texture->width == framebuffer->width);
    assert(texture->height == framebuffer->height);
    assert(texture->format == FORMAT_LDR);

    for (i = 0; i < num_pixels; i++) {
        unsigned char *src_pixel = &framebuffer->color_buffer[i * 4];
        unsigned char *dst_pixel = &texture->ldr_buffer[i * 4];
        dst_pixel[0] = src_pixel[0];
        dst_pixel[1] = src_pixel[1];
        dst_pixel[2] = src_pixel[2];
        dst_pixel[3] = src_pixel[3];
    }
}

void texture_from_depthbuffer(texture_t *texture, framebuffer_t *framebuffer) {
    int num_pixels = texture->width * texture->height;
    int i;

    assert(texture->width == framebuffer->width);
    assert(texture->height == framebuffer->height);
    assert(texture->format == FORMAT_HDR);

    for (i = 0; i < num_pixels; i++) {
        float *src_pixel = &framebuffer->depth_buffer[i];
        float *dst_pixel = &texture->hdr_buffer[i * 4];
        dst_pixel[0] = src_pixel[0];
        dst_pixel[1] = src_pixel[0];
        dst_pixel[2] = src_pixel[0];
        dst_pixel[3] = 1;
    }
}

static vec4_t get_texture_sample(texture_t *texture, float u, float v) {
    int col = (int)((texture->width - 1) * u);
    int row = (int)((texture->height - 1) * v);
    int index = (row * texture->width + col) * 4;
    float r, g, b, a;
    if (texture->format == FORMAT_LDR) {
        r = float_from_uchar(texture->ldr_buffer[index + 0]);
        g = float_from_uchar(texture->ldr_buffer[index + 1]);
        b = float_from_uchar(texture->ldr_buffer[index + 2]);
        a = float_from_uchar(texture->ldr_buffer[index + 3]);
    } else {
        r = texture->hdr_buffer[index + 0];
        g = texture->hdr_buffer[index + 1];
        b = texture->hdr_buffer[index + 2];
        a = texture->hdr_buffer[index + 3];
    }
    return vec4_new(r, g, b, a);
}

vec4_t texture_repeat_sample(texture_t *texture, vec2_t texcoord) {
    float u = texcoord.x - (float)floor(texcoord.x);
    float v = texcoord.y - (float)floor(texcoord.y);
    return get_texture_sample(texture, u, v);
}

vec4_t texture_clamp_sample(texture_t *texture, vec2_t texcoord) {
    float u = float_saturate(texcoord.x);
    float v = float_saturate(texcoord.y);
    return get_texture_sample(texture, u, v);
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
