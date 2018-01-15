#include "camera.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "geometry.h"
#include "platform.h"

/* global constants */

static const float PI = 3.141592653589793f;
static const vec3_t WORLD_UP = {0.0f, 1.0f, 0.0f};

static const float MOVE_SPEED = 2.5f;
static const float ROTATE_SPEED = 0.1f;
static const float ZOOM_SPEED = 1.0f;

static const float PITCH_UPPER = 89.0f;
static const float PITCH_LOWER = -89.0f;

static const float FOVY_DEFAULT = 45.0f;
static const float FOVY_MINIMUM = 15.0f;

static const float DEPTH_NEAR = 0.1f;
static const float DEPTH_FAR = 100.0f;

/* data structure */

struct camera {
    vec3_t position;
    vec3_t front;
    vec3_t right;
    vec3_t up;
    float pitch;
    float yaw;
    float fovy;
    int rotating;
    int last_x_pos;
    int last_y_pos;
    camopt_t options;
};

/* common helper functions */

static float degree_to_radian(float degree) {
    return degree * PI / 180.0f;
}

static float radian_to_degree(float radian) {
    return radian * 180.0f / PI;
}

static float max_float(float a, float b) {
    return a > b ? a : b;
}

static float min_float(float a, float b) {
    return a < b ? a : b;
}

/* camera creating/releasing */

static float calculate_pitch(vec3_t front) {
    /* calculate the angle between front and WORLD_UP */
    float angle = (float)acos(vec3_dot(front, WORLD_UP));
    /* map the angle from [0, PI] to [PI/2, -PI/2] */
    float pitch = PI / 2.0f - angle;
    return radian_to_degree(pitch);
}

static float calculate_yaw(vec3_t front) {
    float yaw = (float)atan2(front.z, front.x);
    return radian_to_degree(yaw);
}

static camopt_t get_default_options(float aspect) {
    camopt_t options;

    options.move_speed   = MOVE_SPEED;
    options.rotate_speed = ROTATE_SPEED;
    options.zoom_speed   = ZOOM_SPEED;

    options.pitch_upper  = PITCH_UPPER;
    options.pitch_lower  = PITCH_LOWER;

    options.fovy_default = FOVY_DEFAULT;
    options.fovy_minimum = FOVY_MINIMUM;

    options.aspect       = aspect;
    options.z_near       = DEPTH_NEAR;
    options.z_far        = DEPTH_FAR;

    return options;
}

camera_t *camera_new(vec3_t position, vec3_t forward, float aspect) {
    camera_t *camera = (camera_t*)malloc(sizeof(camera_t));
    assert(vec3_length(forward) > 1.0e-6f);
    assert(vec3_length(vec3_cross(forward, WORLD_UP)) > 1.0e-6f);
    assert(aspect > 0);

    camera->position   = position;

    camera->front      = vec3_normalize(forward);
    camera->right      = vec3_cross(camera->front, WORLD_UP);
    camera->up         = vec3_cross(camera->right, camera->front);

    camera->pitch      = calculate_pitch(camera->front);
    camera->yaw        = calculate_yaw(camera->front);
    camera->fovy       = FOVY_DEFAULT;

    camera->rotating   = 0;
    camera->last_x_pos = -1;
    camera->last_y_pos = -1;

    camera->options    = get_default_options(aspect);

    return camera;
}

void camera_free(camera_t *camera) {
    free(camera);
}

/* camera customizing */

camopt_t camera_get_options(camera_t *camera) {
    return camera->options;
}

void camera_set_options(camera_t *camera, camopt_t options) {
    camera->options = options;
}

/* input processing */

static void update_orien_vectors(camera_t *camera) {
    float yaw = degree_to_radian(camera->yaw);
    float pitch = degree_to_radian(camera->pitch);
    float sin_yaw = (float)sin(yaw);
    float cos_yaw = (float)cos(yaw);
    float sin_pitch = (float)sin(pitch);
    float cos_pitch = (float)cos(pitch);
    camera->front.x = cos_yaw * cos_pitch;
    camera->front.y = sin_pitch;
    camera->front.z = sin_yaw * cos_pitch;
    camera->front = vec3_normalize(camera->front);
    camera->right = vec3_cross(camera->front, WORLD_UP);
    camera->up = vec3_cross(camera->right, camera->front);
}

static void rotate_camera(camera_t *camera, window_t *window, float dtime) {
    if (input_button_pressed(window, BUTTON_L)) {
        int x_pos, y_pos;
        input_query_cursor(window, &x_pos, &y_pos);
        if (camera->rotating) {
            camopt_t options = camera->options;
            int x_offset = x_pos - camera->last_x_pos;
            int y_offset = y_pos - camera->last_y_pos;
            camera->yaw += x_offset * options.rotate_speed * dtime;
            camera->pitch += y_offset * options.rotate_speed * dtime;
            camera->pitch = min_float(camera->pitch, options.pitch_upper);
            camera->pitch = max_float(camera->pitch, options.pitch_lower);
            update_orien_vectors(camera);
        } else {
            camera->rotating = 1;
        }
        camera->last_x_pos = x_pos;
        camera->last_y_pos = y_pos;
    } else {
        camera->rotating = 0;
    }
}

static void zoom_camera(camera_t *camera, window_t *window, float dtime) {
    camopt_t options = camera->options;
    if (input_button_pressed(window, BUTTON_R)) {
        camera->fovy -= options.zoom_speed * dtime;
        camera->fovy = max_float(camera->fovy, options.fovy_minimum);
    } else {
        camera->fovy += options.zoom_speed * dtime;
        camera->fovy = min_float(camera->fovy, options.fovy_default);
    }
}

static void move_camera(camera_t *camera, window_t *window, float dtime) {
    vec3_t direction = vec3_new(0.0f, 0.0f, 0.0f);
    if (input_key_pressed(window, KEY_A)) {
        direction = vec3_sub(direction, camera->right);
    }
    if (input_key_pressed(window, KEY_D)) {
        direction = vec3_add(direction, camera->right);
    }
    if (input_key_pressed(window, KEY_S)) {
        direction = vec3_sub(direction, camera->front);
    }
    if (input_key_pressed(window, KEY_W)) {
        direction = vec3_add(direction, camera->front);
    }

    if (vec3_length(direction) > 1.0e-6f) {
        float distance = camera->options.move_speed * dtime;
        vec3_t movement = vec3_scale(vec3_normalize(direction), distance);
        camera->position = vec3_add(camera->position, movement);
    }
}

void camera_process_input(camera_t *camera, window_t *window, float dtime) {
    rotate_camera(camera, window, dtime);
    zoom_camera(camera, window, dtime);
    move_camera(camera, window, dtime);
}

/* matrices retrieving */

mat4_t camera_get_view_matrix(camera_t *camera) {
    vec3_t eye = camera->position;
    vec3_t center = vec3_add(eye, camera->front);
    vec3_t up = WORLD_UP;
    return mat4_lookat(eye, center, up);
}

mat4_t camera_get_proj_matrix(camera_t *camera) {
    camopt_t options = camera->options;
    return mat4_perspective(camera->fovy, options.aspect,
                            options.z_near, options.z_far);
}

mat4_t camera_get_viewproj_matrix(camera_t *camera) {
    mat4_t view_matrix = camera_get_view_matrix(camera);
    mat4_t proj_matrix = camera_get_proj_matrix(camera);
    return mat4_mul_mat4(proj_matrix, view_matrix);
}
