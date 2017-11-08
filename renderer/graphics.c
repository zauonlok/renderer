#include <assert.h>
#include <stdlib.h>
#include "graphics.h"
#include "geometry.h"
#include "image.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/*
 * for lookat, projection and viewport matrices, see
 * https://github.com/ssloy/tinyrenderer/wiki/Lesson-4:-Perspective-projection
 * https://github.com/ssloy/tinyrenderer/wiki/Lesson-5:-Moving-the-camera
 */

mat4f_t gfx_lookat_matrix(vec3f_t eye, vec3f_t center, vec3f_t up) {
    vec3f_t zaxis = vec3f_normalize(vec3f_sub(eye, center));
    vec3f_t xaxis = vec3f_normalize(vec3f_cross(up, zaxis));
    vec3f_t yaxis = vec3f_normalize(vec3f_cross(zaxis, xaxis));

    int i;
    mat4f_t model_view = mat4f_identity();
    float xaxis_arr[3], yaxis_arr[3], zaxis_arr[3], center_arr[3];
    vec3f_to_array(xaxis, xaxis_arr);
    vec3f_to_array(yaxis, yaxis_arr);
    vec3f_to_array(zaxis, zaxis_arr);
    vec3f_to_array(center, center_arr);
    for (i = 0; i < 3; i++) {
        model_view.m[0][i] = xaxis_arr[i];
        model_view.m[1][i] = yaxis_arr[i];
        model_view.m[2][i] = zaxis_arr[i];
        model_view.m[i][3] = -center_arr[i];
    }
    return model_view;
}

mat4f_t gfx_projection_matrix(float coeff) {
    mat4f_t projection = mat4f_identity();
    projection.m[3][2] = coeff;
    return projection;
}

mat4f_t gfx_viewport_matrix(int x, int y, int width, int height) {
    const static float depth = 255.0f;
    mat4f_t viewport = mat4f_identity();
    viewport.m[0][0] = width / 2.0f;
    viewport.m[0][3] = x + width / 2.0f;
    viewport.m[1][1] = height / 2.0f;
    viewport.m[1][3] = y + height / 2.0f;
    viewport.m[2][2] = depth / 2.0f;
    viewport.m[2][3] = depth / 2.0f;
    return viewport;
}

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
static vec3f_t calculate_weights(vec2f_t A, vec2f_t B, vec2f_t C, vec2f_t P) {
    vec2f_t AB = vec2f_sub(B, A);
    vec2f_t AC = vec2f_sub(C, A);
    vec2f_t AP = vec2f_sub(P, A);

    float denom = AB.x * AC.y - AB.y * AC.x;
    float s = (AC.y * AP.x - AC.x * AP.y) / denom;
    float t = (AB.x * AP.y - AB.y * AP.x) / denom;

    return vec3f_new(1.0f - s - t, s, t);
}

typedef struct {int min_x, min_y, max_x, max_y;} box_t;

static box_t find_bounding_box(int width, int height,
                               vec2f_t P0, vec2f_t P1, vec2f_t P2) {
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
    vec2f_t screen_points[3];
    box_t box;

    for (i = 0; i < 3; i++) {
        vec4f_t ndc_coord = program->vertex_shader(i, varyings, uniforms);
        screen_coords[i] = mat4f_mul_vec4f(viewport, ndc_coord);

        screen_coords[i].x /= screen_coords[i].w;
        screen_coords[i].y /= screen_coords[i].w;
        screen_coords[i].z /= screen_coords[i].w;
        screen_coords[i].w /= screen_coords[i].w;

        screen_points[i] = vec2f_new(screen_coords[i].x, screen_coords[i].y);
    }

    box = find_bounding_box(width, height, screen_points[0],
                            screen_points[1], screen_points[2]);
    for (x = box.min_x; x <= box.max_x; x++) {
        for (y = box.min_y; y <= box.max_y; y++) {
            vec2f_t point = vec2f_new(x, y);
            vec3f_t weights = calculate_weights(screen_points[0],
                                                screen_points[1],
                                                screen_points[2],
                                                point);
            if (weights.x >= 0 && weights.y >= 0 && weights.z >= 0) {
                float depth = screen_coords[0].z * weights.x +
                              screen_coords[1].z * weights.y +
                              screen_coords[2].z * weights.z;
                int index = y * width + x;
                if (context->depthbuffer[index] < depth) {
                    color_t color;
                    program->interp_varyings(weights, varyings);
                    color = program->fragment_shader(varyings, uniforms);
                    image_set_color(context->framebuffer, y, x);
                    context->depthbuffer[index] = depth;
                }
            }
        }
    }
}
