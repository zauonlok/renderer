#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "graphics.h"
#include "macro.h"
#include "maths.h"

/* framebuffer management */

framebuffer_t *framebuffer_create(int width, int height) {
    int color_buffer_size = width * height * 4;
    int depth_buffer_size = sizeof(float) * width * height;
    vec4_t default_color = {0, 0, 0, 1};
    float default_depth = 1;
    framebuffer_t *framebuffer;

    assert(width > 0 && height > 0);

    framebuffer = (framebuffer_t*)malloc(sizeof(framebuffer_t));
    framebuffer->width = width;
    framebuffer->height = height;
    framebuffer->color_buffer = (unsigned char*)malloc(color_buffer_size);
    framebuffer->depth_buffer = (float*)malloc(depth_buffer_size);

    framebuffer_clear_color(framebuffer, default_color);
    framebuffer_clear_depth(framebuffer, default_depth);

    return framebuffer;
}

void framebuffer_release(framebuffer_t *framebuffer) {
    free(framebuffer->color_buffer);
    free(framebuffer->depth_buffer);
    free(framebuffer);
}

void framebuffer_clear_color(framebuffer_t *framebuffer, vec4_t color) {
    int num_pixels = framebuffer->width * framebuffer->height;
    int i;
    for (i = 0; i < num_pixels; i++) {
        framebuffer->color_buffer[i * 4 + 0] = float_to_uchar(color.x);
        framebuffer->color_buffer[i * 4 + 1] = float_to_uchar(color.y);
        framebuffer->color_buffer[i * 4 + 2] = float_to_uchar(color.z);
        framebuffer->color_buffer[i * 4 + 3] = float_to_uchar(color.w);
    }
}

void framebuffer_clear_depth(framebuffer_t *framebuffer, float depth) {
    int num_pixels = framebuffer->width * framebuffer->height;
    int i;
    for (i = 0; i < num_pixels; i++) {
        framebuffer->depth_buffer[i] = depth;
    }
}

/* program management */

#define MAX_VARYINGS 10

struct program {
    vertex_shader_t *vertex_shader;
    fragment_shader_t *fragment_shader;
    int sizeof_attribs;
    int sizeof_varyings;
    int sizeof_uniforms;
    int double_sided;
    int enable_blend;
    /* for shaders */
    void *shader_attribs[3];
    void *shader_varyings;
    void *shader_uniforms;
    /* for clipping */
    vec4_t in_coords[MAX_VARYINGS];
    vec4_t out_coords[MAX_VARYINGS];
    void *in_varyings[MAX_VARYINGS];
    void *out_varyings[MAX_VARYINGS];
};

program_t *program_create(
        vertex_shader_t *vertex_shader, fragment_shader_t *fragment_shader,
        int sizeof_attribs, int sizeof_varyings, int sizeof_uniforms,
        int double_sided, int enable_blend) {
    program_t *program;
    int i;

    assert(sizeof_attribs > 0 && sizeof_varyings > 0 && sizeof_uniforms > 0);
    assert(sizeof_varyings % sizeof(float) == 0);

    program = (program_t*)malloc(sizeof(program_t));

    program->vertex_shader = vertex_shader;
    program->fragment_shader = fragment_shader;
    program->sizeof_attribs = sizeof_attribs;
    program->sizeof_varyings = sizeof_varyings;
    program->sizeof_uniforms = sizeof_uniforms;
    program->double_sided = double_sided;
    program->enable_blend = enable_blend;

    for (i = 0; i < 3; i++) {
        program->shader_attribs[i] = malloc(sizeof_attribs);
        memset(program->shader_attribs[i], 0, sizeof_attribs);
    }
    program->shader_varyings = malloc(sizeof_varyings);
    memset(program->shader_varyings, 0, sizeof_varyings);
    program->shader_uniforms = malloc(sizeof_uniforms);
    memset(program->shader_uniforms, 0, sizeof_uniforms);
    for (i = 0; i < MAX_VARYINGS; i++) {
        program->in_varyings[i] = malloc(sizeof_varyings);
        memset(program->in_varyings[i], 0, sizeof_varyings);
        program->out_varyings[i] = malloc(sizeof_varyings);
        memset(program->out_varyings[i], 0, sizeof_varyings);
    }

    return program;
}

void program_release(program_t *program) {
    int i;
    for (i = 0; i < 3; i++) {
        free(program->shader_attribs[i]);
    }
    free(program->shader_varyings);
    free(program->shader_uniforms);
    for (i = 0; i < MAX_VARYINGS; i++) {
        free(program->in_varyings[i]);
        free(program->out_varyings[i]);
    }
    free(program);
}

void *program_get_attribs(program_t *program, int nth_vertex) {
    assert(nth_vertex >= 0 && nth_vertex < 3);
    return program->shader_attribs[nth_vertex];
}

void *program_get_uniforms(program_t *program) {
    return program->shader_uniforms;
}

/* graphics pipeline */

/*
 * for triangle clipping, see
 * http://fabiensanglard.net/polygon_codec/
 * http://graphics.idav.ucdavis.edu/education/GraphicsNotes/Clipping.pdf
 */

typedef enum {
    POSITIVE_W,
    POSITIVE_X,
    NEGATIVE_X,
    POSITIVE_Y,
    NEGATIVE_Y,
    POSITIVE_Z,
    NEGATIVE_Z
} plane_t;

static int is_inside_plane(vec4_t coord, plane_t plane) {
    switch (plane) {
        case POSITIVE_W:
            return coord.w >= EPSILON;
        case POSITIVE_X:
            return coord.x <= +coord.w;
        case NEGATIVE_X:
            return coord.x >= -coord.w;
        case POSITIVE_Y:
            return coord.y <= +coord.w;
        case NEGATIVE_Y:
            return coord.y >= -coord.w;
        case POSITIVE_Z:
            return coord.z <= +coord.w;
        case NEGATIVE_Z:
            return coord.z >= -coord.w;
        default:
            assert(0);
            return 0;
    }
}

static float get_intersect_ratio(vec4_t prev, vec4_t curr, plane_t plane) {
    switch (plane) {
        case POSITIVE_W:
            return (prev.w - EPSILON) / (prev.w - curr.w);
        case POSITIVE_X:
            return (prev.w - prev.x) / ((prev.w - prev.x) - (curr.w - curr.x));
        case NEGATIVE_X:
            return (prev.w + prev.x) / ((prev.w + prev.x) - (curr.w + curr.x));
        case POSITIVE_Y:
            return (prev.w - prev.y) / ((prev.w - prev.y) - (curr.w - curr.y));
        case NEGATIVE_Y:
            return (prev.w + prev.y) / ((prev.w + prev.y) - (curr.w + curr.y));
        case POSITIVE_Z:
            return (prev.w - prev.z) / ((prev.w - prev.z) - (curr.w - curr.z));
        case NEGATIVE_Z:
            return (prev.w + prev.z) / ((prev.w + prev.z) - (curr.w + curr.z));
        default:
            assert(0);
            return 0;
    }
}

static int clip_against_plane(
        plane_t plane, int in_num_vertices, int varying_num_floats,
        vec4_t in_coords[MAX_VARYINGS], void *in_varyings[MAX_VARYINGS],
        vec4_t out_coords[MAX_VARYINGS], void *out_varyings[MAX_VARYINGS]) {
    int out_num_vertices = 0;
    int i, j;

    assert(in_num_vertices >= 3 && in_num_vertices <= MAX_VARYINGS);
    for (i = 0; i < in_num_vertices; i++) {
        int prev_index = (i - 1 + in_num_vertices) % in_num_vertices;
        int curr_index = i;
        vec4_t prev_coord = in_coords[prev_index];
        vec4_t curr_coord = in_coords[curr_index];
        float *prev_varyings = (float*)in_varyings[prev_index];
        float *curr_varyings = (float*)in_varyings[curr_index];
        int prev_inside = is_inside_plane(prev_coord, plane);
        int curr_inside = is_inside_plane(curr_coord, plane);

        if (prev_inside != curr_inside) {
            vec4_t *dest_coord = &out_coords[out_num_vertices];
            float *dest_varyings = (float*)out_varyings[out_num_vertices];
            float ratio = get_intersect_ratio(prev_coord, curr_coord, plane);

            *dest_coord = vec4_lerp(prev_coord, curr_coord, ratio);
            /*
             * since this computation is performed in clip space before
             * division by w, clipped varying values are perspective-correct
             */
            for (j = 0; j < varying_num_floats; j++) {
                dest_varyings[j] = float_lerp(prev_varyings[j],
                                              curr_varyings[j],
                                              ratio);
            }
            out_num_vertices += 1;
        }

        if (curr_inside) {
            vec4_t *dest_coord = &out_coords[out_num_vertices];
            float *dest_varyings = (float*)out_varyings[out_num_vertices];
            int sizeof_varyings = varying_num_floats * sizeof(float);

            *dest_coord = curr_coord;
            memcpy(dest_varyings, curr_varyings, sizeof_varyings);
            out_num_vertices += 1;
        }
    }
    assert(out_num_vertices <= MAX_VARYINGS);
    return out_num_vertices;
}

#define CLIP_IN2OUT(plane)                                                  \
    do {                                                                    \
        num_vertices = clip_against_plane(                                  \
            plane, num_vertices, varying_num_floats,                        \
            in_coords, in_varyings, out_coords, out_varyings);              \
        if (num_vertices < 3) {                                             \
            return 0;                                                       \
        }                                                                   \
    } while (0)

#define CLIP_OUT2IN(plane)                                                  \
    do {                                                                    \
        num_vertices = clip_against_plane(                                  \
            plane, num_vertices, varying_num_floats,                        \
            out_coords, out_varyings, in_coords, in_varyings);              \
        if (num_vertices < 3) {                                             \
            return 0;                                                       \
        }                                                                   \
    } while (0)

static int is_vertex_visible(vec4_t v) {
    return fabs(v.x) <= v.w && fabs(v.y) <= v.w && fabs(v.z) <= v.w;
}

static int clip_triangle(
        int sizeof_varyings,
        vec4_t in_coords[MAX_VARYINGS], void *in_varyings[MAX_VARYINGS],
        vec4_t out_coords[MAX_VARYINGS], void *out_varyings[MAX_VARYINGS]) {
    int v0_visible = is_vertex_visible(in_coords[0]);
    int v1_visible = is_vertex_visible(in_coords[1]);
    int v2_visible = is_vertex_visible(in_coords[2]);
    if (v0_visible && v1_visible && v2_visible) {
        out_coords[0] = in_coords[0];
        out_coords[1] = in_coords[1];
        out_coords[2] = in_coords[2];
        memcpy(out_varyings[0], in_varyings[0], sizeof_varyings);
        memcpy(out_varyings[1], in_varyings[1], sizeof_varyings);
        memcpy(out_varyings[2], in_varyings[2], sizeof_varyings);
        return 3;
    } else {
        int varying_num_floats = sizeof_varyings / sizeof(float);
        int num_vertices = 3;
        CLIP_IN2OUT(POSITIVE_W);
        CLIP_OUT2IN(POSITIVE_X);
        CLIP_IN2OUT(NEGATIVE_X);
        CLIP_OUT2IN(POSITIVE_Y);
        CLIP_IN2OUT(NEGATIVE_Y);
        CLIP_OUT2IN(POSITIVE_Z);
        CLIP_IN2OUT(NEGATIVE_Z);
        return num_vertices;
    }
}

/*
 * for facing determination, see subsection 3.5.1 of
 * https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
 *
 * this is the same as (but more efficient than)
 *     vec3_t ab = vec3_sub(b, a);
 *     vec3_t ac = vec3_sub(c, a);
 *     return vec3_cross(ab, ac).z <= 0;
 */
static int is_back_facing(vec3_t ndc_coords[3]) {
    vec3_t a = ndc_coords[0];
    vec3_t b = ndc_coords[1];
    vec3_t c = ndc_coords[2];
    float signed_area = a.x * b.y - a.y * b.x +
                        b.x * c.y - b.y * c.x +
                        c.x * a.y - c.y * a.x;
    return signed_area <= 0;
}

/*
 * for viewport transformation, see subsection 2.12.1 of
 * https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
 */
static vec3_t viewport_transform(int width, int height, vec3_t ndc_coord) {
    float x = (ndc_coord.x + 1) * 0.5f * (float)width;   /* [-1, 1] -> [0, w] */
    float y = (ndc_coord.y + 1) * 0.5f * (float)height;  /* [-1, 1] -> [0, h] */
    float z = (ndc_coord.z + 1) * 0.5f;                  /* [-1, 1] -> [0, 1] */
    return vec3_new(x, y, z);
}

typedef struct {int min_x, min_y, max_x, max_y;} bbox_t;

static int min_integer(int a, int b) {
    return a < b ? a : b;
}

static int max_integer(int a, int b) {
    return a > b ? a : b;
}

static bbox_t find_bounding_box(vec2_t abc[3], int width, int height) {
    vec2_t min = vec2_min(vec2_min(abc[0], abc[1]), abc[2]);
    vec2_t max = vec2_max(vec2_max(abc[0], abc[1]), abc[2]);
    bbox_t bbox;
    bbox.min_x = max_integer((int)floor(min.x), 0);
    bbox.min_y = max_integer((int)floor(min.y), 0);
    bbox.max_x = min_integer((int)ceil(max.x), width - 1);
    bbox.max_y = min_integer((int)ceil(max.y), height - 1);
    return bbox;
}

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
 * for depth interpolation, see subsection 3.5.1 of
 * https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
 */
static float interpolate_depth(float screen_depths[3], vec3_t weights) {
    float depth0 = screen_depths[0] * weights.x;
    float depth1 = screen_depths[1] * weights.y;
    float depth2 = screen_depths[2] * weights.z;
    return depth0 + depth1 + depth2;
}

/*
 * for perspective correct interpolation, see
 * https://www.comp.nus.edu.sg/~lowkl/publications/lowk_persp_interp_techrep.pdf
 * https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
 *
 * equation 15 in reference 1 (page 2) is a simplified 2d version of
 * equation 3.5 in reference 2 (page 58) which uses barycentric coordinates
 */
static void interpolate_varyings(
        void *src_varyings[3], void *dst_varyings,
        int sizeof_varyings, vec3_t weights, float recip_w[3]) {
    int num_floats = sizeof_varyings / sizeof(float);
    float *src0 = (float*)src_varyings[0];
    float *src1 = (float*)src_varyings[1];
    float *src2 = (float*)src_varyings[2];
    float *dst = (float*)dst_varyings;
    float weight0 = recip_w[0] * weights.x;
    float weight1 = recip_w[1] * weights.y;
    float weight2 = recip_w[2] * weights.z;
    float normalizer = 1 / (weight0 + weight1 + weight2);
    int i;
    for (i = 0; i < num_floats; i++) {
        float sum = src0[i] * weight0 + src1[i] * weight1 + src2[i] * weight2;
        dst[i] = sum * normalizer;
    }
}

static void draw_fragment(framebuffer_t *framebuffer, program_t *program,
                          int backface, int index, float depth) {
    vec4_t color;
    int discard;

    /* execute fragment shader */
    discard = 0;
    color = program->fragment_shader(program->shader_varyings,
                                     program->shader_uniforms,
                                     &discard,
                                     backface);
    if (discard) {
        return;
    }
    color = vec4_saturate(color);

    /* perform blending */
    if (program->enable_blend) {
        /* out_color = src_color * src_alpha + dst_color * (1 - src_alpha) */
        unsigned char dst_r = framebuffer->color_buffer[index * 4 + 0];
        unsigned char dst_g = framebuffer->color_buffer[index * 4 + 1];
        unsigned char dst_b = framebuffer->color_buffer[index * 4 + 2];
        color.x = color.x * color.w + float_from_uchar(dst_r) * (1 - color.w);
        color.y = color.y * color.w + float_from_uchar(dst_g) * (1 - color.w);
        color.z = color.z * color.w + float_from_uchar(dst_b) * (1 - color.w);
    }

    /* write color and depth */
    framebuffer->color_buffer[index * 4 + 0] = float_to_uchar(color.x);
    framebuffer->color_buffer[index * 4 + 1] = float_to_uchar(color.y);
    framebuffer->color_buffer[index * 4 + 2] = float_to_uchar(color.z);
    framebuffer->depth_buffer[index] = depth;
}

static int rasterize_triangle(framebuffer_t *framebuffer, program_t *program,
                              vec4_t clip_coords[3], void *varyings[3]) {
    int width = framebuffer->width;
    int height = framebuffer->height;
    vec3_t ndc_coords[3];
    vec2_t screen_coords[3];
    float screen_depths[3];
    float recip_w[3];
    int backface;
    bbox_t bbox;
    int i, x, y;

    /* perspective division */
    for (i = 0; i < 3; i++) {
        vec3_t clip_coord = vec3_from_vec4(clip_coords[i]);
        ndc_coords[i] = vec3_div(clip_coord, clip_coords[i].w);
    }

    /* back-face culling */
    backface = is_back_facing(ndc_coords);
    if (backface && !program->double_sided) {
        return 1;
    }

    /* reciprocals of w */
    for (i = 0; i < 3; i++) {
        recip_w[i] = 1 / clip_coords[i].w;
    }

    /* viewport mapping */
    for (i = 0; i < 3; i++) {
        vec3_t window_coord = viewport_transform(width, height, ndc_coords[i]);
        screen_coords[i] = vec2_new(window_coord.x, window_coord.y);
        screen_depths[i] = window_coord.z;
    }

    /* perform rasterization */
    bbox = find_bounding_box(screen_coords, width, height);
    for (x = bbox.min_x; x <= bbox.max_x; x++) {
        for (y = bbox.min_y; y <= bbox.max_y; y++) {
            vec2_t point = vec2_new((float)x + 0.5f, (float)y + 0.5f);
            vec3_t weights = calculate_weights(screen_coords, point);
            int weight0_okay = weights.x > -EPSILON;
            int weight1_okay = weights.y > -EPSILON;
            int weight2_okay = weights.z > -EPSILON;
            if (weight0_okay && weight1_okay && weight2_okay) {
                int index = y * width + x;
                float depth = interpolate_depth(screen_depths, weights);
                /* early depth testing */
                if (depth <= framebuffer->depth_buffer[index]) {
                    interpolate_varyings(varyings, program->shader_varyings,
                                         program->sizeof_varyings,
                                         weights, recip_w);
                    draw_fragment(framebuffer, program, backface, index, depth);
                }
            }
        }
    }

    return 0;
}

void graphics_draw_triangle(framebuffer_t *framebuffer, program_t *program) {
    int num_vertices;
    int i;

    /* execute vertex shader */
    for (i = 0; i < 3; i++) {
        vec4_t clip_coord = program->vertex_shader(program->shader_attribs[i],
                                                   program->in_varyings[i],
                                                   program->shader_uniforms);
        program->in_coords[i] = clip_coord;
    }

    /* triangle clipping */
    num_vertices = clip_triangle(program->sizeof_varyings,
                                 program->in_coords, program->in_varyings,
                                 program->out_coords, program->out_varyings);

    /* triangle assembly */
    for (i = 0; i < num_vertices - 2; i++) {
        int index0 = 0;
        int index1 = i + 1;
        int index2 = i + 2;
        vec4_t clip_coords[3];
        void *varyings[3];
        int is_culled;

        clip_coords[0] = program->out_coords[index0];
        clip_coords[1] = program->out_coords[index1];
        clip_coords[2] = program->out_coords[index2];
        varyings[0] = program->out_varyings[index0];
        varyings[1] = program->out_varyings[index1];
        varyings[2] = program->out_varyings[index2];

        is_culled = rasterize_triangle(framebuffer, program,
                                       clip_coords, varyings);
        if (is_culled) {
            break;
        }
    }
}
