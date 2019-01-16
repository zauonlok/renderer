#include "graphics.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "geometry.h"
#include "image.h"
#include "mesh.h"

/* framebuffer management */

static const vec4_t DEFAULT_COLOR = {0, 0, 0, 1};
static const float DEFAULT_DEPTH = 1;

framebuffer_t *framebuffer_create(int width, int height) {
    framebuffer_t *framebuffer;
    colorbuffer_t *colorbuffer;
    depthbuffer_t *depthbuffer;
    int num_elems = width * height;

    assert(width > 0 && height > 0);

    colorbuffer = (colorbuffer_t*)malloc(sizeof(colorbuffer_t));
    colorbuffer->width  = width;
    colorbuffer->height = height;
    colorbuffer->buffer = (vec4_t*)malloc(sizeof(vec4_t) * num_elems);

    depthbuffer = (depthbuffer_t*)malloc(sizeof(depthbuffer_t));
    depthbuffer->width  = width;
    depthbuffer->height = height;
    depthbuffer->buffer = (float*)malloc(sizeof(float) * num_elems);

    framebuffer = (framebuffer_t*)malloc(sizeof(framebuffer_t));
    framebuffer->width       = width;
    framebuffer->height      = height;
    framebuffer->colorbuffer = colorbuffer;
    framebuffer->depthbuffer = depthbuffer;

    framebuffer_clear_color(framebuffer, DEFAULT_COLOR);
    framebuffer_clear_depth(framebuffer, DEFAULT_DEPTH);

    return framebuffer;
}

void framebuffer_release(framebuffer_t *framebuffer) {
    free(framebuffer->colorbuffer->buffer);
    free(framebuffer->colorbuffer);
    free(framebuffer->depthbuffer->buffer);
    free(framebuffer->depthbuffer);
    free(framebuffer);
}

void framebuffer_clear_color(framebuffer_t *framebuffer, vec4_t color) {
    colorbuffer_t *colorbuffer = framebuffer->colorbuffer;
    int num_elems = colorbuffer->width * colorbuffer->height;
    int i;
    for (i = 0; i < num_elems; i++) {
        colorbuffer->buffer[i] = color;
    }
}

void framebuffer_clear_depth(framebuffer_t *framebuffer, float depth) {
    depthbuffer_t *depthbuffer = framebuffer->depthbuffer;
    int num_elems = depthbuffer->width * depthbuffer->height;
    int i;
    for (i = 0; i < num_elems; i++) {
        depthbuffer->buffer[i] = depth;
    }
}

/* program management */

program_t *program_create(
        vertex_shader_t *vertex_shader, fragment_shader_t *fragment_shader,
        int sizeof_attribs, int sizeof_varyings, int sizeof_uniforms) {
    program_t *program;
    int i;

    program = (program_t*)malloc(sizeof(program_t));
    for (i = 0; i < 3; i++) {
        program->attribs[i] = malloc(sizeof_attribs);
        memset(program->attribs[i], 0, sizeof_attribs);
    }
    for (i = 0; i < 4; i++) {
        program->varyings[i] = malloc(sizeof_varyings);
        memset(program->varyings[i], 0, sizeof_varyings);
    }
    program->uniforms = malloc(sizeof_uniforms);
    memset(program->uniforms, 0, sizeof_uniforms);

    program->vertex_shader   = vertex_shader;
    program->fragment_shader = fragment_shader;
    program->sizeof_attribs  = sizeof_attribs;
    program->sizeof_varyings = sizeof_varyings;
    program->sizeof_uniforms = sizeof_uniforms;

    return program;
}

void program_release(program_t *program) {
    int i;
    for (i = 0; i < 3; i++) {
        free(program->attribs[i]);
    }
    for (i = 0; i < 4; i++) {
        free(program->varyings[i]);
    }
    free(program->uniforms);
    free(program);
}

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

vec4_t texture_sample(texture_t *texture, vec2_t texcoord) {
    float u = texcoord.x;
    float v = texcoord.y;
    if (u < 0 || v < 0 || u > 1 || v > 1) {
        vec4_t blank = {0, 0, 0, 0};
        return blank;
    } else {
        int c = (int)((texture->width - 1) * u + 0.5);
        int r = (int)((texture->height - 1) * v + 0.5);
        int index = r * texture->width + c;
        return texture->buffer[index];
    }
}

/* triangle rasterization */

/*
 * for barycentric coordinates, see
 * http://blackpawn.com/texts/pointinpoly/
 *
 * solve
 *     P = A + s * AB + t * AC  -->  AP = s * AB + t * AC
 * then
 *     s = (AC.y * AP.x - AC.x * AP.y) / (AB.x * AC.y - AB.y * AC.x)
 *     t = (AB.x * AP.y - AB.y * AP.x) / (AB.x * AC.y - AB.y * AC.x)
 *
 * if s < 0 or t < 0, we've walked in the wrong direction
 * if s > 1 or t > 1, we've walked too far in a direction
 * if s + t > 1, we've crossed the edge BC
 *
 * P in ABC if and only if (s >= 0) && (t >= 0) && (1 - s - t >= 0)
 *
 * notice
 *     P = A + s * AB + t * AC
 *       = A + s * (B - A) + t * (C - A)
 *       = (1 - s - t) * A + s * B + t * C
 * then
 *     weight_A = 1 - s - t
 *     weight_B = s
 *     weight_C = t
 */
static vec3_t calculate_weights(vec2_t abc[3], vec2_t p) {
    vec2_t a = abc[0];
    vec2_t b = abc[1];
    vec2_t c = abc[2];
    vec2_t ab = vec2_sub(b, a);
    vec2_t ac = vec2_sub(c, a);
    vec2_t ap = vec2_sub(p, a);
    float factor = 1 / (ab.x * ac.y - ab.y * ac.x);
    float s = (ac.y * ap.x - ac.x * ap.y) * factor;
    float t = (ab.x * ap.y - ab.y * ap.x) * factor;
    vec3_t weights = vec3_new(1 - s - t, s, t);
    return weights;
}

/*
 * for triangle clipping, see
 * https://www.gamasutra.com/view/news/168577/
 */
static int is_vertex_invisible(vec4_t clip_coord) {
    float x = clip_coord.x;
    float y = clip_coord.y;
    float z = clip_coord.z;
    float w = clip_coord.w;
    return (x < -w || x > w || y < -w || y > w || z < -w || z > w || w <= 0);
}

static int is_back_facing(vec3_t ndc_coords[3]) {
    vec3_t a = ndc_coords[0];
    vec3_t b = ndc_coords[1];
    vec3_t c = ndc_coords[2];
    vec3_t ab = vec3_sub(b, a);
    vec3_t ac = vec3_sub(c, a);
    return vec3_cross(ab, ac).z < 0;
}

/*
 * for viewport transform, see
 * http://docs.gl/gl2/glViewport
 * http://docs.gl/gl2/glDepthRange
 */
static vec3_t viewport_transform(int width_, int height_, vec3_t ndc_coord) {
    float width = (float)width_;
    float height = (float)height_;
    float x = (ndc_coord.x + 1) * 0.5f * width;   /* [-1, 1] -> [0, w] */
    float y = (ndc_coord.y + 1) * 0.5f * height;  /* [-1, 1] -> [0, h] */
    float z = (ndc_coord.z + 1) * 0.5f;           /* [-1, 1] -> [0, 1] */
    return vec3_new(x, y, z);
}

static float min_float(float a, float b, float c, float lower_bound) {
    float min = a;
    min = (b < min) ? b : min;
    min = (c < min) ? c : min;
    min = (min < lower_bound) ? lower_bound : min;
    return min;
}

static float max_float(float a, float b, float c, float upper_bound) {
    float max = a;
    max = (b > max) ? b : max;
    max = (c > max) ? c : max;
    max = (max > upper_bound) ? upper_bound : max;
    return max;
}

typedef struct {vec2_t min; vec2_t max;} bbox_t;

static bbox_t find_bounding_box(vec2_t abc[3], int width, int height) {
    vec2_t a = abc[0];
    vec2_t b = abc[1];
    vec2_t c = abc[2];
    bbox_t bbox;
    bbox.min.x = min_float(a.x, b.x, c.x, 0);
    bbox.min.y = min_float(a.y, b.y, c.y, 0);
    bbox.max.x = max_float(a.x, b.x, c.x, (float)(width - 1));
    bbox.max.y = max_float(a.y, b.y, c.y, (float)(height - 1));
    return bbox;
}


static float calculate_depth(float screen_depths[3], vec3_t weights) {
    float depth0 = screen_depths[0];
    float depth1 = screen_depths[1];
    float depth2 = screen_depths[2];
    return depth0 * weights.x + depth1 * weights.y + depth2 * weights.z;
}

/*
 * for perspective-correct interpolation, see
 * https://www.comp.nus.edu.sg/~lowkl/publications/lowk_persp_interp_techrep.pdf
 * https://www.khronos.org/registry/OpenGL/specs/gl/glspec33.core.pdf
 *
 * equation 15 in reference 1 (page 2) is a simplified 2d version of
 * equation 3.9 in reference 2 (page 117) which uses barycentric coordinates
 */
static void interpolate_varyings(void *varyings[4], int sizeof_varyings,
                                 float recip_w[3], vec3_t weights) {
    int num_floats = sizeof_varyings / sizeof(float);
    float *src0 = (float*)varyings[0];
    float *src1 = (float*)varyings[1];
    float *src2 = (float*)varyings[2];
    float *dst = (float*)varyings[3];
    float weight0 = recip_w[0] * weights.x;
    float weight1 = recip_w[1] * weights.y;
    float weight2 = recip_w[2] * weights.z;
    float normalizer = 1 / (weight0 + weight1 + weight2);
    int i;
    assert(num_floats * (int)sizeof(float) == sizeof_varyings);
    for (i = 0; i < num_floats; i++) {
        float sum = src0[i] * weight0 + src1[i] * weight1 + src2[i] * weight2;
        dst[i] = sum * normalizer;
    }
}

void graphics_draw_triangle(framebuffer_t *framebuffer, program_t *program) {
    colorbuffer_t *colorbuffer = framebuffer->colorbuffer;
    depthbuffer_t *depthbuffer = framebuffer->depthbuffer;
    int width = framebuffer->width;
    int height = framebuffer->height;
    vec4_t clip_coords[3];
    vec3_t ndc_coords[3];
    vec2_t screen_coords[3];
    float screen_depths[3];
    float recip_w[3];
    bbox_t bbox;
    int i, x, y;

    /* calculate clip coordinates */
    for (i = 0; i < 3; i++) {
        void *attribs = program->attribs[i];
        void *varyings = program->varyings[i];
        void *uniforms = program->uniforms;
        clip_coords[i] = program->vertex_shader(attribs, varyings, uniforms);
    }

    /* naive view volume culling */
    for (i = 0; i < 3; i++) {
        if (is_vertex_invisible(clip_coords[i])) {
            return;
        }
    }

    /* perspective division */
    for (i = 0; i < 3; i++) {
        vec3_t clip_coord = vec3_from_vec4(clip_coords[i]);
        ndc_coords[i] = vec3_div(clip_coord, clip_coords[i].w);
    }

    /* back-face culling */
    if (is_back_facing(ndc_coords)) {
        return;
    }

    /* calculate reciprocals of w */
    for (i = 0; i < 3; i++) {
        recip_w[i] = 1 / clip_coords[i].w;
    }

    /* calculate screen coordinates */
    for (i = 0; i < 3; i++) {
        vec3_t window_coords = viewport_transform(width, height, ndc_coords[i]);
        screen_coords[i] = vec2_new(window_coords.x, window_coords.y);
        screen_depths[i] = window_coords.z;
    }

    /* perform rasterization */
    bbox = find_bounding_box(screen_coords, width, height);
    for (x = (int)bbox.min.x; x <= bbox.max.x; x++) {
        for (y = (int)bbox.min.y; y <= bbox.max.y; y++) {
            vec2_t point = vec2_new((float)x, (float)y);
            vec3_t weights = calculate_weights(screen_coords, point);
            if (weights.x >= 0 && weights.y >= 0 && weights.z >= 0) {
                int index = y * width + x;
                float depth = calculate_depth(screen_depths, weights);
                /* early depth testing */
                if (depth <= depthbuffer->buffer[index]) {
                    int sizeof_varyings = program->sizeof_varyings;
                    vec4_t color;
                    interpolate_varyings(program->varyings, sizeof_varyings,
                                         recip_w, weights);
                    color = program->fragment_shader(program->varyings[3],
                                                     program->uniforms);
                    colorbuffer->buffer[index] = vec4_saturate(color);
                    depthbuffer->buffer[index] = depth;
                }
            }
        }
    }
}
