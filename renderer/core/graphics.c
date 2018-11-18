#include "graphics.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "geometry.h"
#include "image.h"
#include "mesh.h"

/* rendertarget management */

rendertarget_t *rendertarget_create(int width, int height) {
    rendertarget_t *rendertarget;
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

    rendertarget = (rendertarget_t*)malloc(sizeof(rendertarget_t));
    rendertarget->width       = width;
    rendertarget->height      = height;
    rendertarget->colorbuffer = colorbuffer;
    rendertarget->depthbuffer = depthbuffer;

    rendertarget_clear(rendertarget, CLEAR_COLOR | CLEAR_DEPTH);
    return rendertarget;
}

void rendertarget_release(rendertarget_t *rendertarget) {
    free(rendertarget->colorbuffer->buffer);
    free(rendertarget->colorbuffer);
    free(rendertarget->depthbuffer->buffer);
    free(rendertarget->depthbuffer);
    free(rendertarget);
}

void rendertarget_clear(rendertarget_t *rendertarget, clearmask_t clearmask) {
    int num_elems = rendertarget->width * rendertarget->height;
    int i;

    if (clearmask & CLEAR_COLOR) {
        colorbuffer_t *colorbuffer = rendertarget->colorbuffer;
        vec4_t default_color = {0, 0, 0, 1};
        for (i = 0; i < num_elems; i++) {
            colorbuffer->buffer[i] = default_color;
        }
    }
    if (clearmask & CLEAR_DEPTH) {
        depthbuffer_t *depthbuffer = rendertarget->depthbuffer;
        float default_depth = 1;
        for (i = 0; i < num_elems; i++) {
            depthbuffer->buffer[i] = default_depth;
        }
    }
}

/* triangle rasterization */

typedef struct {int valid; vec2_t ab, ac; float factor;} cache_t;

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
 * note
 *     P = A + s * AB + t * AC
 *       = A + s * (B - A) + t * (C - A)
 *       = (1 - s - t) * A + s * B + t * C
 * then
 *     weight_A = 1 - s - t
 *     weight_B = s
 *     weight_C = t
 */
static vec3_t calculate_weights(vec2_t abc[3], vec2_t p, cache_t *cache) {
    vec2_t a = abc[0];
    vec2_t b = abc[1];
    vec2_t c = abc[2];

    vec2_t ab, ac, ap;
    float factor, s, t;
    vec3_t weights;

    if (cache->valid) {
        ab = cache->ab;
        ac = cache->ac;
        factor = cache->factor;
    } else {
        ab = cache->ab = vec2_sub(b, a);
        ac = cache->ac = vec2_sub(c, a);
        factor = cache->factor = 1 / (ab.x * ac.y - ab.y * ac.x);
        cache->valid = 1;
    }

    ap = vec2_sub(p, a);
    s = (ac.y * ap.x - ac.x * ap.y) * factor;
    t = (ab.x * ap.y - ab.y * ap.x) * factor;
    weights = vec3_new(1 - s - t, s, t);
    return weights;
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

typedef struct {float min_x, min_y, max_x, max_y;} box_t;

static box_t find_bounding_box(vec2_t abc[3], int width, int height) {
    vec2_t a = abc[0];
    vec2_t b = abc[1];
    vec2_t c = abc[2];
    box_t box;
    box.min_x = min_float(a.x, b.x, c.x, 0);
    box.min_y = min_float(a.y, b.y, c.y, 0);
    box.max_x = max_float(a.x, b.x, c.x, (float)(width - 1));
    box.max_y = max_float(a.y, b.y, c.y, (float)(height - 1));
    return box;
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
    int is_clockwise = vec3_cross(ab, ac).z < 0;
    return is_clockwise;
}

/*
 * for viewport transform, see
 * http://docs.gl/gl2/glViewport
 * http://docs.gl/gl2/glDepthRange
 * http://www.songho.ca/opengl/gl_transform.html
 */
static vec3_t viewport_transform(int width_, int height_, vec3_t ndc_coord) {
    float width = (float)width_;
    float height = (float)height_;
    float x = (ndc_coord.x + 1) * 0.5f * width;   // [-1, 1] -> [0, 1] -> [0, w]
    float y = (ndc_coord.y + 1) * 0.5f * height;  // [-1, 1] -> [0, 1] -> [0, h]
    float z = (ndc_coord.z + 1) * 0.5f;           // [-1, 1] -> [0, 1]
    return vec3_new(x, y, z);
}

static float calculate_depth(float screen_depths[3], vec3_t weights) {
    float depth0 = screen_depths[0];
    float depth1 = screen_depths[1];
    float depth2 = screen_depths[2];
    return depth0 * weights.x + depth1 * weights.y + depth2 * weights.z;
}

static void interp_varyings(program_t *program, vec3_t weights) {
    int num_floats = program->sizeof_varyings / sizeof(float);
    float *src0 = (float*)program->varyings[0];
    float *src1 = (float*)program->varyings[1];
    float *src2 = (float*)program->varyings[2];
    float *dst = (float*)program->varyings[3];
    int i;
    assert(num_floats * (int)sizeof(float) == program->sizeof_varyings);
    for (i = 0; i < num_floats; i++) {
        dst[i] = 0;
        dst[i] += src0[i] * weights.x;
        dst[i] += src1[i] * weights.y;
        dst[i] += src2[i] * weights.z;
    }
}

void graphics_draw_triangle(rendertarget_t *rendertarget, program_t *program) {
    int width = rendertarget->width;
    int height = rendertarget->height;
    vec4_t clip_coords[3];
    vec3_t ndc_coords[3];
    vec2_t screen_points[3];
    float screen_depths[3];
    cache_t cache;
    box_t box;
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
        float factor = 1 / clip_coords[i].w;
        ndc_coords[i] = vec3_scale(vec3_from_vec4(clip_coords[i]), factor);
    }

    /* back-face culling */
    if (is_back_facing(ndc_coords)) {
        return;
    }

    /* calculate screen coordinates */
    for (i = 0; i < 3; i++) {
        vec3_t screen_coord = viewport_transform(width, height, ndc_coords[i]);
        screen_points[i] = vec2_new(screen_coord.x, screen_coord.y);
        screen_depths[i] = screen_coord.z;
    }

    /* perform rasterization */
    cache.valid = 0;
    box = find_bounding_box(screen_points, width, height);
    for (x = (int)box.min_x; x <= box.max_x; x++) {
        for (y = (int)box.min_y; y <= box.max_y; y++) {
            vec2_t point = vec2_new((float)x, (float)y);
            vec3_t weights = calculate_weights(screen_points, point, &cache);
            if (weights.x >= 0 && weights.y >= 0 && weights.z >= 0) {
                depthbuffer_t *depthbuffer = rendertarget->depthbuffer;
                int index = y * width + x;
                float depth = calculate_depth(screen_depths, weights);
                /* early depth testing */
                if (depthbuffer->buffer[index] >= depth) {
                    colorbuffer_t *colorbuffer = rendertarget->colorbuffer;
                    void *varyings = program->varyings[3];
                    void *uniforms = program->uniforms;
                    vec4_t color;
                    interp_varyings(program, weights);
                    color = program->fragment_shader(varyings, uniforms);
                    colorbuffer->buffer[index] = vec4_saturate(color);
                    depthbuffer->buffer[index] = depth;
                }
            }
        }
    }
}

/* texture management */

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
                float luminance = pixel[0] / 255.0f;
                *texel = vec4_new(luminance, luminance, luminance, 1);
            } else if (channels == 2) {  /* GL_LUMINANCE_ALPHA */
                float luminance = pixel[0] / 255.0f;
                float alpha = pixel[1] / 255.0f;
                *texel = vec4_new(luminance, luminance, luminance, alpha);
            } else if (channels == 3) {  /* GL_RGB */
                float blue = pixel[0] / 255.0f;
                float green = pixel[1] / 255.0f;
                float red = pixel[2] / 255.0f;
                *texel = vec4_new(red, green, blue, 1);
            } else if (channels == 4) {  /* GL_RGBA */
                float blue = pixel[0] / 255.0f;
                float green = pixel[1] / 255.0f;
                float red = pixel[2] / 255.0f;
                float alpha = pixel[3] / 255.0f;
                *texel = vec4_new(red, green, blue, alpha);
            } else {
                assert(0);
            }
        }
    }

    return texture;
}

texture_t *texture_from_colorbuffer(colorbuffer_t *colorbuffer) {
    int width = colorbuffer->width;
    int height = colorbuffer->height;
    int num_elems = width * height;
    texture_t *texture;

    texture = (texture_t*)malloc(sizeof(texture_t));
    texture->width  = width;
    texture->height = height;
    texture->buffer = (vec4_t*)malloc(sizeof(vec4_t) * num_elems);
    memcpy(texture->buffer, colorbuffer->buffer, sizeof(vec4_t) * num_elems);

    return texture;
}

texture_t *texture_from_depthbuffer(depthbuffer_t *depthbuffer) {
    int width = depthbuffer->width;
    int height = depthbuffer->height;
    texture_t *texture;
    int r, c;

    texture = (texture_t*)malloc(sizeof(texture_t));
    texture->width  = width;
    texture->height = height;
    texture->buffer = (vec4_t*)malloc(sizeof(vec4_t) * width * height);

    for (r = 0; r < height; r++) {
        for (c = 0; c < width; c++) {
            int index = r * width + c;
            float depth = depthbuffer->buffer[index];
            texture->buffer[index] = vec4_new(depth, depth, depth, 1);
        }
    }

    return texture;
}

void texture_release(texture_t *texture) {
    free(texture->buffer);
    free(texture);
}

vec4_t texture_sample(texture_t *texture, float u, float v) {
    if (u < 0 || v < 0 || u > 1 || v > 1) {
        vec4_t empty = {0, 0, 0, 0};
        return empty;
    } else {
        int c = (int)((texture->width - 1) * u + 0.5f);
        int r = (int)((texture->height - 1) * v + 0.5f);
        int index = r * texture->width + c;
        return texture->buffer[index];
    }
}

/* private blit functions */

void colorbuffer_blit_bgr(colorbuffer_t *src, image_t *dst) {
    int width = src->width < dst->width ? src->width : dst->width;
    int height = src->height < dst->height ? src->height : dst->height;
    int r, c;

    assert(width > 0 && height > 0);
    assert(dst->channels == 3 || dst->channels == 4);

    for (r = 0; r < height; r++) {
        for (c = 0; c < width; c++) {
            int src_index = (src->height - 1 - r) * src->width + c;  /* flip */
            int dst_index = r * dst->width * dst->channels + c * dst->channels;
            vec4_t src_value = src->buffer[src_index];
            unsigned char *dst_pixel = &(dst->buffer[dst_index]);
            dst_pixel[2] = (unsigned char)(src_value.x * 255);  /* red */
            dst_pixel[1] = (unsigned char)(src_value.y * 255);  /* green */
            dst_pixel[0] = (unsigned char)(src_value.z * 255);  /* blue */
        }
    }
}

void colorbuffer_blit_rgb(colorbuffer_t *src, image_t *dst) {
    int width = src->width < dst->width ? src->width : dst->width;
    int height = src->height < dst->height ? src->height : dst->height;
    int r, c;

    assert(width > 0 && height > 0);
    assert(dst->channels == 3 || dst->channels == 4);

    for (r = 0; r < height; r++) {
        for (c = 0; c < width; c++) {
            int src_index = (src->height - 1 - r) * src->width + c;  /* flip */
            int dst_index = r * dst->width * dst->channels + c * dst->channels;
            vec4_t src_value = src->buffer[src_index];
            unsigned char *dst_pixel = &(dst->buffer[dst_index]);
            dst_pixel[0] = (unsigned char)(src_value.x * 255);  /* red */
            dst_pixel[1] = (unsigned char)(src_value.y * 255);  /* green */
            dst_pixel[2] = (unsigned char)(src_value.z * 255);  /* blue */
        }
    }
}
