#include "camera.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "geometry.h"
#include "platform.h"

static const vec3_t WORLD_UP = {0, 1, 0};

static const float MOVE_SPEED = 3;
static const float ROTATE_SPEED = 3;
static const float ZOOM_SPEED = 60;

static const float PITCH_UPPER = 89;
static const float PITCH_LOWER = -89;

static const float FOVY_DEFAULT = 45;
static const float FOVY_MINIMUM = 25;

static const float Z_NEAR = 1;
static const float Z_FAR = 1000;

struct camera {
    vec3_t position;
    float yaw;          /* h angle */
    float pitch;        /* v angle */
    float fovy;
    /* derived */
    vec3_t forward;     /* -z axis */
    vec3_t right;       /* +x axis */
    vec3_t up;          /* +y axis */
    /* history */
    int rotating;
    int last_xpos;
    int last_ypos;
    /* options */
    camopt_t options;
};

/* camera creating/releasing */

static float pitch_from_forward(vec3_t forward) {
    float angle = (float)acos(forward.y);
    float pitch = PI / 2 - angle;  /* [0, PI] -> [PI/2, -PI/2] */
    return pitch * TO_DEGREES;
}

static float yaw_from_forward(vec3_t forward) {
    float yaw = (float)atan2(forward.x, forward.z);
    return yaw * TO_DEGREES;
}

static vec3_t forward_from_yaw_pitch(float yaw_, float pitch_) {
    float yaw = yaw_ * TO_RADIANS;
    float pitch = pitch_ * TO_RADIANS;
    float sin_yaw = (float)sin(yaw);
    float cos_yaw = (float)cos(yaw);
    float sin_pitch = (float)sin(pitch);
    float cos_pitch = (float)cos(pitch);
    float x = cos_pitch * sin_yaw;
    float y = sin_pitch;
    float z = cos_pitch * cos_yaw;
    return vec3_new(x, y, z);
}

static void update_basis_vectors(camera_t *camera) {
    camera->forward = forward_from_yaw_pitch(camera->yaw, camera->pitch);
    camera->right = vec3_cross(camera->forward, WORLD_UP);
    camera->up = vec3_cross(camera->right, camera->forward);
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
    options.z_near       = Z_NEAR;
    options.z_far        = Z_FAR;

    return options;
}

camera_t *camera_create(vec3_t position, vec3_t forward_, float aspect) {
    vec3_t forward;
    camera_t *camera;

    assert(vec3_length(forward_) > 1e-5f);
    assert(vec3_length(vec3_cross(forward_, WORLD_UP)) > 1e-5f);
    assert(aspect > 0);

    forward = vec3_normalize(forward_);
    camera = (camera_t*)malloc(sizeof(camera_t));

    camera->position  = position;
    camera->yaw       = yaw_from_forward(forward);
    camera->pitch     = pitch_from_forward(forward);
    camera->fovy      = FOVY_DEFAULT;

    camera->rotating  = 0;
    camera->last_xpos = 0;
    camera->last_ypos = 0;

    camera->options   = get_default_options(aspect);

    update_basis_vectors(camera);

    return camera;
}

void camera_release(camera_t *camera) {
    free(camera);
}

/* camera customizing */

camopt_t camera_get_options(camera_t *camera) {
    return camera->options;
}

void camera_set_options(camera_t *camera, camopt_t options) {
    assert(options.pitch_upper >= options.pitch_lower);
    assert(options.pitch_upper < 90 && options.pitch_lower > -90);
    assert(options.fovy_default >= options.fovy_minimum);
    assert(options.fovy_minimum > 0 && options.aspect > 0);
    assert(options.z_far > options.z_near && options.z_near > 0);
    camera->options = options;
}

/* input processing */

static float clamp_float(float f, float min, float max) {
    assert(min <= max);
    return (f < min) ? min : ((f > max) ? max : f);
}

static void zoom_camera(camera_t *camera, window_t *window, float delta_time) {
    camopt_t options = camera->options;
    float fovy_min = options.fovy_minimum;
    float fovy_max = options.fovy_default;
    float amount = options.zoom_speed * delta_time;
    if (input_button_pressed(window, BUTTON_R)) {
        camera->fovy -= amount;
    } else {
        camera->fovy += amount;
    }
    camera->fovy = clamp_float(camera->fovy, fovy_min, fovy_max);
}

static void rotate_camera(camera_t *camera, window_t *window,
                          float delta_time) {
    if (input_button_pressed(window, BUTTON_L)) {
        int xpos, ypos;
        input_query_cursor(window, &xpos, &ypos);
        if (camera->rotating) {
            camopt_t options = camera->options;
            float pitch_min = options.pitch_lower;
            float pitch_max = options.pitch_upper;
            float factor = options.rotate_speed * delta_time;
            int h_offset = xpos - camera->last_xpos;
            int v_offset = ypos - camera->last_ypos;
            camera->yaw += h_offset * factor;
            camera->pitch += v_offset * factor;
            camera->pitch = clamp_float(camera->pitch, pitch_min, pitch_max);
            update_basis_vectors(camera);
        } else {
            camera->rotating = 1;
        }
        camera->last_xpos = xpos;
        camera->last_ypos = ypos;
    } else {
        camera->rotating = 0;
    }
}

static void move_camera(camera_t *camera, window_t *window, float delta_time) {
    vec3_t accumulation = vec3_new(0, 0, 0);
    if (input_key_pressed(window, KEY_A)) {
        accumulation = vec3_sub(accumulation, camera->right);
    }
    if (input_key_pressed(window, KEY_D)) {
        accumulation = vec3_add(accumulation, camera->right);
    }
    if (input_key_pressed(window, KEY_S)) {
        accumulation = vec3_sub(accumulation, camera->forward);
    }
    if (input_key_pressed(window, KEY_W)) {
        accumulation = vec3_add(accumulation, camera->forward);
    }

    if (vec3_length(accumulation) > 1e-5f) {
        vec3_t direction = vec3_normalize(accumulation);
        float distance = camera->options.move_speed * delta_time;
        vec3_t movement = vec3_mul(direction, distance);
        camera->position = vec3_add(camera->position, movement);
    }
}

void camera_process_input(camera_t *camera, window_t *window,
                          float delta_time) {
    zoom_camera(camera, window, delta_time);
    rotate_camera(camera, window, delta_time);
    move_camera(camera, window, delta_time);
}

/* property retrieving */

vec3_t camera_get_position(camera_t *camera) {
    return camera->position;
}

vec3_t camera_get_forward(camera_t *camera) {
    return camera->forward;
}

mat4_t camera_get_view_matrix(camera_t *camera) {
    vec3_t eye = camera->position;
    vec3_t target = vec3_add(eye, camera->forward);
    vec3_t up = WORLD_UP;
    return mat4_lookat(eye, target, up);
}

mat4_t camera_get_proj_matrix(camera_t *camera) {
    float fovy = camera->fovy * TO_RADIANS;
    camopt_t options = camera->options;
    float aspect = options.aspect;
    float near = options.z_near;
    float far = options.z_far;
    return mat4_perspective(fovy, aspect, near, far);
}
