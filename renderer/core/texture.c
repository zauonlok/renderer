#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graphics.h"
#include "image.h"
#include "macro.h"
#include "maths.h"
#include "texture.h"

/* radiance hdr related functions */

static void read_line(FILE *file, char line[LINE_SIZE]) {
    if (fgets(line, LINE_SIZE, file) == NULL) {
        assert(0);
    }
}

static int starts_with(const char *string, const char *prefix) {
    return strncmp(string, prefix, strlen(prefix)) == 0;
}

static void read_hdr_header(FILE *file, int *width, int *height) {
    char line[LINE_SIZE];
    int header_found = 0;
    int format_found = 0;
    int items;

    read_line(file, line);
    assert(starts_with(line, "#?"));

    while (1) {
        read_line(file, line);
        if (strlen(line) == 1 && line[0] == '\n') {
            header_found = 1;
            break;
        } else if (starts_with(line, "FORMAT=")) {
            assert(starts_with(line, "FORMAT=32-bit_rle_rgbe"));
            format_found = 1;
        } else if (starts_with(line, "GAMMA=")) {
            /* ignore, for now */
        } else if (starts_with(line, "EXPOSURE=")) {
            /* ignore, for now */
        } else if (starts_with(line, "#")) {
            /* ignore comments */
        } else {
            assert(0);
        }
    }
    assert(header_found != 0 && format_found != 0);

    read_line(file, line);
    items = sscanf(line, "-Y %d +X %d", height, width);
    assert(items == 2 && *width > 0 && *height > 0);
}

static vec4_t texel_from_rgbe(unsigned char rgbe[4]) {
    float rm = rgbe[0];  /* red mantissa */
    float gm = rgbe[1];  /* green mantissa */
    float bm = rgbe[2];  /* blue mantissa */
    float eb = rgbe[3];  /* exponent biased */
    if (eb == 0) {
        return vec4_new(0, 0, 0, 1);
    } else {
        float ev = eb - 128;     /* exponent value */
        float factor = (float)((1.0 / 256) * pow(2, ev));
        float rv = rm * factor;  /* red value */
        float gv = gm * factor;  /* green value */
        float bv = bm * factor;  /* blue value */
        return vec4_new(rv, gv, bv, 1);
    }
}

static unsigned char read_byte(FILE *file) {
    int byte = fgetc(file);
    assert(byte != EOF);
    return (unsigned char)byte;
}

static void read_flat_scanline(FILE *file, texture_t *texture, int row) {
    int width = texture->width;
    int i, j;
    for (i = 0; i < width; i++) {
        int index = row * width + i;
        unsigned char rgbe[4];
        for (j = 0; j < 4; j++) {
            rgbe[j] = read_byte(file);
        }
        texture->buffer[index] = texel_from_rgbe(rgbe);
    }
}

static void read_rle_scanline(FILE *file, texture_t *texture, int row) {
    int width = texture->width;
    unsigned char *channels[4];
    int i, j;

    for (i = 0; i < 4; i++) {
        channels[i] = (unsigned char*)malloc(width);
    }
    for (i = 0; i < 4; i++) {
        int size = 0;
        while (size < width) {
            unsigned char byte = read_byte(file);
            if (byte > 128) {
                int count = byte - 128;
                unsigned char value = read_byte(file);
                assert(count > 0 && size + count <= width);
                for (j = 0; j < count; j++) {
                    channels[i][size++] = value;
                }
            } else {
                int count = byte;
                assert(count > 0 && size + count <= width);
                for (j = 0; j < count; j++) {
                    channels[i][size++] = read_byte(file);
                }
            }
        }
        assert(size == width);
    }

    for (i = 0; i < width; i++) {
        int index = row * width + i;
        unsigned char rgbe[4];
        for (j = 0; j < 4; j++) {
            rgbe[j] = channels[j][i];
        }
        texture->buffer[index] = texel_from_rgbe(rgbe);
    }
    for (i = 0; i < 4; i++) {
        free(channels[i]);
    }
}

static void read_hdr_scanline(FILE *file, texture_t *texture, int row) {
    int width = texture->width;
    if (width < 8 || width > 0x7fff) {
        read_flat_scanline(file, texture, row);
    } else {
        unsigned char bytes[4];
        int i;
        for (i = 0; i < 4; i++) {
            bytes[i] = read_byte(file);
        }
        if (bytes[0] != 2 || bytes[1] != 2 || bytes[2] & 0x80) {
            fseek(file, -4, SEEK_CUR);
            read_flat_scanline(file, texture, row);
        } else {
            assert(bytes[2] * 256 + bytes[3] == width);
            read_rle_scanline(file, texture, row);
        }
    }
}

static texture_t *load_hdr(const char *filename) {
    texture_t *texture;
    int width, height;
    FILE *file;
    int i;

    file = fopen(filename, "rb");
    assert(file != NULL);
    read_hdr_header(file, &width, &height);
    texture = texture_create(width, height);
    for (i = 0; i < height; i++) {
        int row = height - 1 - i;
        read_hdr_scanline(file, texture, row);
    }
    fclose(file);

    return texture;
}

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

static const char *extract_extension(const char *filename) {
    const char *dot_pos = strrchr(filename, '.');
    return dot_pos == NULL ? "" : dot_pos + 1;
}

texture_t *texture_from_file(const char *filename) {
    const char *extension = extract_extension(filename);
    if (strcmp(extension, "hdr") == 0) {
        return load_hdr(filename);
    } else {
        image_t *image = image_load(filename);
        texture_t *texture = texture_from_image(image);
        image_release(image);
        return texture;
    }
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
            int img_index = (r * width + c) * channels;
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
        vec4_t texel = texture->buffer[i];
        texture->buffer[i] = vec4_srgb2linear(texel);
    }
}

void texture_linear2srgb(texture_t *texture) {
    int num_elems = texture->width * texture->height;
    int i;
    for (i = 0; i < num_elems; i++) {
        vec4_t texel = texture->buffer[i];
        texture->buffer[i] = vec4_linear2srgb(texel);
    }
}

void texture_flip_h(texture_t *texture) {
    int half_width = texture->width / 2;
    int r, c;
    for (r = 0; r < texture->height; r++) {
        for (c = 0; c < half_width; c++) {
            int flipped_c = texture->width - 1 - c;
            int index1 = r * texture->width + c;
            int index2 = r * texture->width + flipped_c;
            vec4_t temp = texture->buffer[index1];
            texture->buffer[index1] = texture->buffer[index2];
            texture->buffer[index2] = temp;
        }
    }
}

void texture_flip_v(texture_t *texture) {
    int half_height = texture->height / 2;
    int r, c;
    for (r = 0; r < half_height; r++) {
        for (c = 0; c < texture->width; c++) {
            int flipped_r = texture->height - 1 - r;
            int index1 = r * texture->width + c;
            int index2 = flipped_r * texture->width + c;
            vec4_t temp = texture->buffer[index1];
            texture->buffer[index1] = texture->buffer[index2];
            texture->buffer[index2] = temp;
        }
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
                              const char *positive_z, const char *negative_z) {
    cubemap_t *cubemap;
    int i;

    cubemap = (cubemap_t*)malloc(sizeof(cubemap_t));
    cubemap->faces[0] = texture_from_file(positive_x);
    cubemap->faces[1] = texture_from_file(negative_x);
    cubemap->faces[2] = texture_from_file(positive_y);
    cubemap->faces[3] = texture_from_file(negative_y);
    cubemap->faces[4] = texture_from_file(positive_z);
    cubemap->faces[5] = texture_from_file(negative_z);

    /*
     * for face uv origin, see
     * https://stackoverflow.com/questions/11685608/convention-of-faces-in-opengl-cubemapping
     */
    for (i = 0; i < 6; i++) {
        texture_flip_v(cubemap->faces[i]);
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

void cubemap_linear2srgb(cubemap_t *cubemap) {
    int i;
    for (i = 0; i < 6; i++) {
        texture_linear2srgb(cubemap->faces[i]);
    }
}

/*
 * for sampling cubemap, see subsection 3.7.5 of
 * https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
 */
static int select_cubemap_face(vec3_t direction, vec2_t *texcoord) {
    float abs_x = (float)fabs(direction.x);
    float abs_y = (float)fabs(direction.y);
    float abs_z = (float)fabs(direction.z);
    float ma, sc, tc;
    int face_index;

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

    texcoord->x = (sc / ma + 1) / 2;
    texcoord->y = (tc / ma + 1) / 2;
    return face_index;
}

vec4_t cubemap_repeat_sample(cubemap_t *cubemap, vec3_t direction) {
    vec2_t texcoord;
    int face_index = select_cubemap_face(direction, &texcoord);
    return texture_repeat_sample(cubemap->faces[face_index], texcoord);
}

vec4_t cubemap_clamp_sample(cubemap_t *cubemap, vec3_t direction) {
    vec2_t texcoord;
    int face_index = select_cubemap_face(direction, &texcoord);
    return texture_clamp_sample(cubemap->faces[face_index], texcoord);
}

vec4_t cubemap_sample(cubemap_t *cubemap, vec3_t direction) {
    return cubemap_repeat_sample(cubemap, direction);
}
