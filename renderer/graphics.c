#include <assert.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include "graphics.h"
#include "geometry.h"
#include "image.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/* context management */

context_t *gfx_create_context(int width, int height) {
    context_t *context = (context_t*)malloc(sizeof(context_t));
    context->framebuffer = image_create(width, height, 3);
    context->depthbuffer = (float*)malloc(sizeof(float) * width * height);
    context->viewport    = gfx_viewport_matrix(0, 0, width, height);
    gfx_clear_buffers(context);
    return context;
}

void gfx_release_context(context_t *context) {
    image_release(context->framebuffer);
    free(context->depthbuffer);
    free(context);
}

void gfx_clear_buffers(context_t *context) {
    image_t *framebuffer = context->framebuffer;
    float *depthbuffer = context->depthbuffer;
    int width = framebuffer->width;
    int height = framebuffer->height;
    int i;
    memset(framebuffer->buffer, 0, width * height * framebuffer->channels);
    for (i = 0; i < width * height; i++) {
        depthbuffer[i] = FLT_MIN;
    }
}

/* triangle rasterization */

/*
 * for barycentric coordinates, see http://blackpawn.com/texts/pointinpoly/
 * solve P = A + s * AB + t * AC
 * --> AP = s * AB + t * AC
 * --> s = (AC.y * AP.x - AC.x * AP.y) / (AB.x * AC.y - AB.y * AC.x)
 * --> t = (AB.x * AP.y - AB.y * AP.x) / (AB.x * AC.y - AB.y * AC.x)
 *
 * if s < 0 or t < 0 then we've walked in the wrong direction
 * if s > 1 or t > 1 then we've walked too far in a direction
 * if s + t > 1 then we've crossed the edge BC
 * therefore P is in ABC only if (s >= 0) && (t >= 0) && (1 - s - t >= 0)
 *
 * note P = A + s * AB + t * AC
 *        = A + s * (B - A) + t * (C - A)
 *        = (1 - s - t) * A + s * B + t * C
 */
static vec3f_t calculate_weights(vec2_t A, vec2_t B, vec2_t C, vec2_t P) {
    vec2_t AB = vec2_sub(B, A);
    vec2_t AC = vec2_sub(C, A);
    vec2_t AP = vec2_sub(P, A);

    float denom = AB.x * AC.y - AB.y * AC.x;
    float s = (AC.y * AP.x - AC.x * AP.y) / denom;
    float t = (AB.x * AP.y - AB.y * AP.x) / denom;

    return vec3f_new(1.0f - s - t, s, t);
}

typedef struct {float min_x, min_y, max_x, max_y;} box_t;

static box_t find_bounding_box(int width, int height,
                               vec2_t P0, vec2_t P1, vec2_t P2) {
    box_t box;

    box.min_x = MIN(P1.x, P0.x);
    box.min_y = MIN(P1.y, P0.y);
    box.max_x = MAX(P1.x, P0.x);
    box.max_y = MAX(P1.y, P0.y);

    box.min_x = MIN(P2.x, box.min_x);
    box.min_y = MIN(P2.y, box.min_y);
    box.max_x = MAX(P2.x, box.max_x);
    box.max_y = MAX(P2.y, box.max_y);

    box.min_x = MAX(0, box.min_x);
    box.min_y = MAX(0, box.min_y);
    box.max_x = MIN(width - 1, box.max_x);
    box.max_y = MIN(height - 1, box.max_y);

    return box;
}

void gfx_draw_triangle(context_t *context, program_t *program) {
    /* for convenience */
    int width = context->framebuffer->width;
    int height = context->framebuffer->height;
    mat4f_t viewport = context->viewport;
    void *varyings = program->varyings;
    void *uniforms = program->uniforms;

    int i, x, y;
    vec4f_t screen_coords[3];
    vec2_t screen_points[3];
    box_t box;

    for (i = 0; i < 3; i++) {
        vec4f_t clip_coord = program->vertex_shader(i, varyings, uniforms);
        vec4f_t ndc_coord = vec4f_scale(clip_coord, 1.0f / clip_coord.w);
        screen_coords[i] = mat4f_mul_vec4f(viewport, ndc_coord);
        screen_points[i] = vec2_new(screen_coords[i].x, screen_coords[i].y);
    }

    box = find_bounding_box(width, height, screen_points[0],
                            screen_points[1], screen_points[2]);
    for (x = (int)box.min_x; x <= box.max_x; x++) {
        for (y = (int)box.min_y; y <= box.max_y; y++) {
            vec2_t point = vec2_new((float)x, (float)y);
            vec3f_t weights = calculate_weights(screen_points[0],
                                                screen_points[1],
                                                screen_points[2],
                                                point);
            if (weights.x >= 0 && weights.y >= 0 && weights.z >= 0) {
                float depth = screen_coords[0].z * weights.x
                              + screen_coords[1].z * weights.y
                              + screen_coords[2].z * weights.z;
                int index = y * width + x;
                if (context->depthbuffer[index] < depth) {
                    color_t color;
                    program->interp_varyings(weights, varyings);
                    color = program->fragment_shader(varyings, uniforms);
                    image_set_color(context->framebuffer, y, x, color);
                    context->depthbuffer[index] = depth;
                }
            }
        }
    }
}

/* common matrices */

/*
 * for lookat, projection and viewport matrices, see
 * https://github.com/ssloy/tinyrenderer/wiki/Lesson-4:-Perspective-projection
 * https://github.com/ssloy/tinyrenderer/wiki/Lesson-5:-Moving-the-camera
 * 3D Math Primer for Graphics and Game Development, Chapter 10
 */

mat4f_t gfx_lookat_matrix(vec3f_t eye, vec3f_t center, vec3f_t up) {
    vec3f_t zaxis = vec3f_normalize(vec3f_sub(eye, center));
    vec3f_t xaxis = vec3f_normalize(vec3f_cross(up, zaxis));
    vec3f_t yaxis = vec3f_normalize(vec3f_cross(zaxis, xaxis));

    int i;
    mat4f_t lookat = mat4f_identity();
    float xaxis_arr[3], yaxis_arr[3], zaxis_arr[3], center_arr[3];
    vec3f_to_array(xaxis, xaxis_arr);
    vec3f_to_array(yaxis, yaxis_arr);
    vec3f_to_array(zaxis, zaxis_arr);
    vec3f_to_array(center, center_arr);
    for (i = 0; i < 3; i++) {
        lookat.m[0][i] = xaxis_arr[i];
        lookat.m[1][i] = yaxis_arr[i];
        lookat.m[2][i] = zaxis_arr[i];
        lookat.m[i][3] = -center_arr[i];
    }
    return lookat;
}

mat4f_t gfx_projection_matrix(float coeff) {
    mat4f_t projection = mat4f_identity();
    projection.m[3][2] = coeff;
    return projection;
}

mat4f_t gfx_viewport_matrix(int x, int y, int width, int height) {
    static const float depth = 255.0f;
    mat4f_t viewport = mat4f_identity();
    viewport.m[0][0] = width / 2.0f;
    viewport.m[0][3] = x + width / 2.0f;
    viewport.m[1][1] = height / 2.0f;
    viewport.m[1][3] = y + height / 2.0f;
    viewport.m[2][2] = depth / 2.0f;
    viewport.m[2][3] = depth / 2.0f;
    return viewport;
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

vec3f_t gfx_sample_normal(image_t *normal_map, vec2_t texcoord) {
    color_t color = gfx_sample_texture(normal_map, texcoord);
    vec3f_t normal;
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

vec2_t gfx_interp_vec2(vec2_t vs[3], vec3f_t weights_) {
    int i;
    vec2_t out = {0.0f, 0.0f};
    float weights[3];
    vec3f_to_array(weights_, weights);
    for (i = 0; i < 3; i++) {
        out.x += vs[i].x * weights[i];
        out.y += vs[i].y * weights[i];
    }
    return out;
}

vec3f_t gfx_interp_vec3f(vec3f_t vs[3], vec3f_t weights_) {
    int i;
    vec3f_t out = {0.0f, 0.0f, 0.0f};
    float weights[3];
    vec3f_to_array(weights_, weights);
    for (i = 0; i < 3; i++) {
        out.x += vs[i].x * weights[i];
        out.y += vs[i].y * weights[i];
        out.z += vs[i].z * weights[i];
    }
    return out;
}

vec4f_t gfx_interp_vec4f(vec4f_t vs[3], vec3f_t weights_) {
    int i;
    vec4f_t out = {0.0f, 0.0f, 0.0f, 0.0f};
    float weights[3];
    vec3f_to_array(weights_, weights);
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
vec3f_t gfx_reflect_light(vec3f_t normal, vec3f_t light) {
    float n_dot_l = vec3f_dot(normal, light);
    vec3f_t two_n_dot = vec3f_scale(normal, n_dot_l * 2.0f);
    return vec3f_normalize(vec3f_sub(two_n_dot, light));
}
