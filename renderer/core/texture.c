#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "geometry.h"
#include "image.h"
#include "texture.h"

struct texture {int width, height; vec4_t *buffer;};
struct cubemap {texture_t *faces[6];};

/* texture management */

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

    texture = (texture_t*)malloc(sizeof(texture_t));
    texture->width  = width;
    texture->height = height;
    texture->buffer = (vec4_t*)malloc(sizeof(vec4_t) * width * height);

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

void texture_release(texture_t *texture) {
    free(texture->buffer);
    free(texture);
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
    int c = (int)((texture->width - 1) * u + 0.5);
    int r = (int)((texture->height - 1) * v + 0.5);
    int index = r * texture->width + c;
    return texture->buffer[index];
}

/* cubemap management */

cubemap_t *cubemap_from_files(const char *positive_x, const char *negative_x,
                              const char *positive_y, const char *negative_y,
                              const char *positive_z, const char *negative_z) {
    cubemap_t *cubemap = (cubemap_t*)malloc(sizeof(cubemap_t));
    cubemap->faces[0] = texture_from_file(positive_x);  /* right */
    cubemap->faces[1] = texture_from_file(negative_x);  /* left */
    cubemap->faces[2] = texture_from_file(positive_y);  /* top */
    cubemap->faces[3] = texture_from_file(negative_y);  /* bottom */
    cubemap->faces[4] = texture_from_file(positive_z);  /* front */
    cubemap->faces[5] = texture_from_file(negative_z);  /* back */
    return cubemap;
}

cubemap_t *cubemap_from_images(image_t *positive_x, image_t *negative_x,
                               image_t *positive_y, image_t *negative_y,
                               image_t *positive_z, image_t *negative_z) {
    cubemap_t *cubemap = (cubemap_t*)malloc(sizeof(cubemap_t));
    cubemap->faces[0] = texture_from_image(positive_x);  /* right */
    cubemap->faces[1] = texture_from_image(negative_x);  /* left */
    cubemap->faces[2] = texture_from_image(positive_y);  /* top */
    cubemap->faces[3] = texture_from_image(negative_y);  /* bottom */
    cubemap->faces[4] = texture_from_image(positive_z);  /* front */
    cubemap->faces[5] = texture_from_image(negative_z);  /* back */
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
    float abs_x = (direction.x > 0) ? direction.x : -direction.x;
    float abs_y = (direction.y > 0) ? direction.y : -direction.y;
    float abs_z = (direction.z > 0) ? direction.z : -direction.z;
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
