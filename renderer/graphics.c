#include "graphics.h"
#include <assert.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include "geometry.h"
#include "image.h"

/* context management */

context_t *gfx_create_context(int width, int height) {
    context_t *context = (context_t*)malloc(sizeof(context_t));
    context->colorbuffer = image_create(width, height, 3);
    context->depthbuffer = (float*)malloc(sizeof(float) * width * height);
    context->viewport    = mat4_viewport(0, 0, width, height);
    gfx_clear_buffers(context);
    return context;
}

void gfx_release_context(context_t *context) {
    image_release(context->colorbuffer);
    free(context->depthbuffer);
    free(context);
}

void gfx_clear_buffers(context_t *context) {
    image_t *colorbuffer = context->colorbuffer;
    float *depthbuffer = context->depthbuffer;
    int width = colorbuffer->width;
    int height = colorbuffer->height;
    int channels = colorbuffer->channels;
    int i;
    memset(colorbuffer->buffer, 0, width * height * channels);
    for (i = 0; i < width * height; i++) {
        depthbuffer[i] = FLT_MAX;
    }
}

/* triangle rasterization */

/*
 * for barycentric coordinates, see http://blackpawn.com/texts/pointinpoly/
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
 * therefore P is in ABC only if (s >= 0) && (t >= 0) && (1 - s - t >= 0)
 *
 * note P = A + s * AB + t * AC
 *        = A + s * (B - A) + t * (C - A)
 *        = (1 - s - t) * A + s * B + t * C
 */
static vec3_t calculate_weights(vec2_t A, vec2_t B, vec2_t C, vec2_t P) {
    vec2_t AB = vec2_sub(B, A);
    vec2_t AC = vec2_sub(C, A);
    vec2_t AP = vec2_sub(P, A);

    float denom = AB.x * AC.y - AB.y * AC.x;
    float s = (AC.y * AP.x - AC.x * AP.y) / denom;
    float t = (AB.x * AP.y - AB.y * AP.x) / denom;

    return vec3_new(1.0f - s - t, s, t);
}

typedef struct {float min_x, min_y, max_x, max_y;} box_t;

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

static box_t find_bounding_box(int width, int height,
                               vec2_t p0, vec2_t p1, vec2_t p2) {
    box_t box;
    box.min_x = min_float(p0.x, p1.x, p2.x, 0.0f);
    box.min_y = min_float(p0.y, p1.y, p2.y, 0.0f);
    box.max_x = max_float(p0.x, p1.x, p2.x, (float)(width - 1));
    box.max_y = max_float(p0.y, p1.y, p2.y, (float)(height - 1));
    return box;
}

void gfx_draw_triangle(context_t *context, program_t *program) {
    /* for convenience */
    image_t *colorbuffer = context->colorbuffer;
    float *depthbuffer = context->depthbuffer;
    mat4_t viewport = context->viewport;
    int width = context->colorbuffer->width;
    int height = context->colorbuffer->height;
    vertex_shader_t *vertex_shader = program->vertex_shader;
    fragment_shader_t *fragment_shader = program->fragment_shader;
    interp_varyings_t *interp_varyings = program->interp_varyings;
    void *attribs = program->attribs;
    void *varyings = program->varyings;
    void *uniforms = program->uniforms;

    int i, x, y;
    vec4_t screen_coords[3];
    vec2_t screen_points[3];
    box_t box;

    for (i = 0; i < 3; i++) {
        vec4_t clip_coord = vertex_shader(i, attribs, varyings, uniforms);
        vec4_t ndc_coord = vec4_scale(clip_coord, 1.0f / clip_coord.w);
        screen_coords[i] = mat4_mul_vec4(viewport, ndc_coord);
        screen_points[i] = vec2_new(screen_coords[i].x, screen_coords[i].y);
    }

    box = find_bounding_box(width, height, screen_points[0],
                            screen_points[1], screen_points[2]);
    for (x = (int)box.min_x; x <= box.max_x; x++) {
        for (y = (int)box.min_y; y <= box.max_y; y++) {
            vec2_t point = vec2_new((float)x, (float)y);
            vec3_t weights = calculate_weights(screen_points[0],
                                               screen_points[1],
                                               screen_points[2],
                                               point);
            if (weights.x >= 0 && weights.y >= 0 && weights.z >= 0) {
                float depth = screen_coords[0].z * weights.x
                              + screen_coords[1].z * weights.y
                              + screen_coords[2].z * weights.z;
                int index = y * width + x;
                if (depthbuffer[index] > depth) {
                    color_t color;
                    interp_varyings(weights, varyings);
                    color = fragment_shader(varyings, uniforms);
                    image_set_color(colorbuffer, y, x, color);
                    depthbuffer[index] = depth;
                }
            }
        }
    }
}

/* texture sampling */

color_t gfx_sample_texture(image_t *texture, vec2_t texcoord) {
    float u = texcoord.x;
    float v = texcoord.y;
    int row, col;
    assert(u >= 0.0f && u <= 1.0f && v >= 0.0f && v <= 1.0f);
    col = (int)((texture->width - 1) * u + 0.5f);
    row = (int)((texture->height - 1) * v + 0.5f);
    return image_get_color(texture, row, col);
}

color_t gfx_sample_diffuse(image_t *diffuse_map, vec2_t texcoord) {
    return gfx_sample_texture(diffuse_map, texcoord);
}

vec3_t gfx_sample_normal(image_t *normal_map, vec2_t texcoord) {
    color_t color = gfx_sample_texture(normal_map, texcoord);
    vec3_t normal;
    /* interpret rgb values as xyz directions */
    normal.x = color.r / 255.0f * 2.0f - 1.0f;
    normal.y = color.g / 255.0f * 2.0f - 1.0f;
    normal.z = color.b / 255.0f * 2.0f - 1.0f;
    return normal;
}

float gfx_sample_specular(image_t *specular_map, vec2_t texcoord) {
    color_t color = gfx_sample_texture(specular_map, texcoord);
    return (float)color.b;
}

/* vector interpolation */

vec2_t gfx_interp_vec2(vec2_t vs[3], vec3_t weights_) {
    int i;
    vec2_t out = {0.0f, 0.0f};
    float weights[3];
    vec3_to_array(weights_, weights);
    for (i = 0; i < 3; i++) {
        out.x += vs[i].x * weights[i];
        out.y += vs[i].y * weights[i];
    }
    return out;
}

vec3_t gfx_interp_vec3(vec3_t vs[3], vec3_t weights_) {
    int i;
    vec3_t out = {0.0f, 0.0f, 0.0f};
    float weights[3];
    vec3_to_array(weights_, weights);
    for (i = 0; i < 3; i++) {
        out.x += vs[i].x * weights[i];
        out.y += vs[i].y * weights[i];
        out.z += vs[i].z * weights[i];
    }
    return out;
}

vec4_t gfx_interp_vec4(vec4_t vs[3], vec3_t weights_) {
    int i;
    vec4_t out = {0.0f, 0.0f, 0.0f, 0.0f};
    float weights[3];
    vec3_to_array(weights_, weights);
    for (i = 0; i < 3; i++) {
        out.x += vs[i].x * weights[i];
        out.y += vs[i].y * weights[i];
        out.z += vs[i].z * weights[i];
        out.w += vs[i].w * weights[i];
    }
    return out;
}

/* utility functions */

/*
 * reflected = 2 * normal * dot(normal, light) - light
 * normal must be normalized
 * light is the inverse of the incident light
 */
vec3_t gfx_reflect_light(vec3_t normal, vec3_t light) {
    float n_dot_l = vec3_dot(normal, light);
    vec3_t two_n_dot = vec3_scale(normal, n_dot_l * 2.0f);
    return vec3_normalize(vec3_sub(two_n_dot, light));
}
