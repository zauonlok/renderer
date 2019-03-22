#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "geometry.h"
#include "graphics.h"
#include "image.h"
#include "texture.h"

struct texture {int width, height; vec4_t *buffer;};
struct cubemap {texture_t *faces[6];};

/* texture related functions */

texture_t *texture_create(int width, int height) {
    int buffer_size = sizeof(vec4_t) * width * height;
    texture_t *texture;

    assert(width > 0 && height > 0);

    texture = (texture_t*)malloc(sizeof(texture_t));
    texture->width  = width;
    texture->height = height;
    texture->buffer = (vec4_t*)malloc(buffer_size);
    memset(texture->buffer, 0, buffer_size);
    return texture;
}

void texture_release(texture_t *texture) {
    free(texture->buffer);
    free(texture);
}

texture_t *texture_from_file(const char *filename) {
    image_t *image = image_load(filename);
    texture_t *texture = texture_from_image(image);
    image_release(image);
    return texture;
}

static float uchar_to_float(unsigned char value) {
    return value / 255.0f;
}

/*
 * for texture formats, see
 * http://docs.gl/gl2/glTexImage2D
 */
texture_t *texture_from_image(image_t *image) {
    int width = image->width;
    int height = image->height;
    int channels = image->channels;
    texture_t *texture;
    int r, c;

    texture = texture_create(width, height);
    for (r = 0; r < height; r++) {
        for (c = 0; c < width; c++) {
            int img_index = r * width * channels + c * channels;
            int tex_index = r * width + c;
            unsigned char *pixel = &image->buffer[img_index];
            vec4_t *texel = &texture->buffer[tex_index];
            if (channels == 1) {         /* GL_LUMINANCE */
                float luminance = uchar_to_float(pixel[0]);
                *texel = vec4_new(luminance, luminance, luminance, 1);
            } else if (channels == 2) {  /* GL_LUMINANCE_ALPHA */
                float luminance = uchar_to_float(pixel[0]);
                float alpha = uchar_to_float(pixel[1]);
                *texel = vec4_new(luminance, luminance, luminance, alpha);
            } else if (channels == 3) {  /* GL_RGB */
                float blue = uchar_to_float(pixel[0]);
                float green = uchar_to_float(pixel[1]);
                float red = uchar_to_float(pixel[2]);
                *texel = vec4_new(red, green, blue, 1);
            } else if (channels == 4) {  /* GL_RGBA */
                float blue = uchar_to_float(pixel[0]);
                float green = uchar_to_float(pixel[1]);
                float red = uchar_to_float(pixel[2]);
                float alpha = uchar_to_float(pixel[3]);
                *texel = vec4_new(red, green, blue, alpha);
            } else {
                assert(0);
            }
        }
    }

    return texture;
}

void texture_from_color(texture_t *texture, framebuffer_t *framebuffer) {
    int num_elems = texture->width * texture->height;
    int i;

    assert(texture->width == framebuffer->width);
    assert(texture->height == framebuffer->height);

    for (i = 0; i < num_elems; i++) {
        texture->buffer[i] = framebuffer->colorbuffer[i];
    }
}

void texture_from_depth(texture_t *texture, framebuffer_t *framebuffer) {
    int num_elems = texture->width * texture->height;
    int i;

    assert(texture->width == framebuffer->width);
    assert(texture->height == framebuffer->height);

    for (i = 0; i < num_elems; i++) {
        float depth = framebuffer->depthbuffer[i];
        texture->buffer[i] = vec4_new(depth, depth, depth, 1);
    }
}

void texture_srgb2linear(texture_t *texture) {
    int num_elems = texture->width * texture->height;
    int i;
    for (i = 0; i < num_elems; i++) {
        vec4_t *texel = &texture->buffer[i];
        texel->x = (float)pow(texel->x, 2.2);
        texel->y = (float)pow(texel->y, 2.2);
        texel->z = (float)pow(texel->z, 2.2);
    }
}

vec4_t texture_sample(texture_t *texture, vec2_t texcoord) {
    float u = texcoord.x - (float)floor(texcoord.x);
    float v = texcoord.y - (float)floor(texcoord.y);
    int c = (int)((texture->width - 1) * u);
    int r = (int)((texture->height - 1) * v);
    int index = r * texture->width + c;
    return texture->buffer[index];
}

/* cubemap related functions */

/*
 * for face uv origin, see
 * https://stackoverflow.com/questions/11685608/
 */
cubemap_t *cubemap_from_files(const char *positive_x, const char *negative_x,
                              const char *positive_y, const char *negative_y,
                              const char *positive_z, const char *negative_z) {
    cubemap_t *cubemap;
    image_t *faces[6];
    int i;

    faces[0] = image_load(positive_x);  /* right */
    faces[1] = image_load(negative_x);  /* left */
    faces[2] = image_load(positive_y);  /* top */
    faces[3] = image_load(negative_y);  /* bottom */
    faces[4] = image_load(positive_z);  /* front */
    faces[5] = image_load(negative_z);  /* back */

    cubemap = (cubemap_t*)malloc(sizeof(cubemap_t));
    for (i = 0; i < 6; i++) {
        image_flip_v(faces[i]);
        cubemap->faces[i] = texture_from_image(faces[i]);
        image_release(faces[i]);
    }

    return cubemap;
}

void cubemap_release(cubemap_t *cubemap) {
    int i;
    for (i = 0; i < 6; i++) {
        texture_release(cubemap->faces[i]);
    }
    free(cubemap);
}

void cubemap_srgb2linear(cubemap_t *cubemap) {
    int i;
    for (i = 0; i < 6; i++) {
        texture_srgb2linear(cubemap->faces[i]);
    }
}

/*
 * for sampling cubemap, see subsection 3.7.5 of
 * https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
 */
vec4_t cubemap_sample(cubemap_t *cubemap, vec3_t direction) {
    float abs_x = (float)fabs(direction.x);
    float abs_y = (float)fabs(direction.y);
    float abs_z = (float)fabs(direction.z);
    int face_index;
    float ma, sc, tc;
    vec2_t texcoord;

    if (abs_x > abs_y && abs_x > abs_z) {  /* major axis -> x */
        ma = abs_x;
        if (direction.x > 0) {  /* positive x */
            face_index = 0;
            sc = -direction.z;
            tc = -direction.y;
        } else {                /* negative x */
            face_index = 1;
            sc = +direction.z;
            tc = -direction.y;
        }
    } else if (abs_y > abs_z) {            /* major axis -> y */
        ma = abs_y;
        if (direction.y > 0) {  /* positive y */
            face_index = 2;
            sc = +direction.x;
            tc = +direction.z;
        } else {                /* negative y */
            face_index = 3;
            sc = +direction.x;
            tc = -direction.z;
        }
    } else {                               /* major axis -> z */
        ma = abs_z;
        if (direction.z > 0) {  /* positive z */
            face_index = 4;
            sc = +direction.x;
            tc = -direction.y;
        } else {                /* negative z */
            face_index = 5;
            sc = -direction.x;
            tc = -direction.y;
        }
    }

    texcoord.x = (sc / ma + 1) / 2;
    texcoord.y = (tc / ma + 1) / 2;

    return texture_sample(cubemap->faces[face_index], texcoord);
}
