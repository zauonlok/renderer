#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "camera.h"
#include "macro.h"
#include "maths.h"

/*
 * for orbital camera controls, see
 * https://github.com/mrdoob/three.js/blob/master/examples/js/controls/OrbitControls.js
 */

static const float NEAR = 0.1f;
static const float FAR = 10000;
static const float FOVY = TO_RADIANS(60);
static const vec3_t UP = {0, 1, 0};

struct camera {
    vec3_t position;
    vec3_t target;
    float aspect;
};

/* camera creating/releasing */

camera_t *camera_create(vec3_t position, vec3_t target, float aspect) {
    camera_t *camera;

    assert(vec3_length(vec3_sub(position, target)) > EPSILON && aspect > 0);

    camera = (camera_t*)malloc(sizeof(camera_t));
    camera->position = position;
    camera->target = target;
    camera->aspect = aspect;

    return camera;
}

void camera_release(camera_t *camera) {
    free(camera);
}

/* camera updating */

void camera_set_transform(camera_t *camera, vec3_t position, vec3_t target) {
    assert(vec3_length(vec3_sub(position, target)) > EPSILON);
    camera->position = position;
    camera->target = target;
}

static vec3_t calculate_pan(vec3_t from_camera, motion_t motion) {
    vec3_t forward = vec3_normalize(from_camera);
    vec3_t left = vec3_cross(UP, forward);
    vec3_t up = vec3_cross(forward, left);

    float distance = vec3_length(from_camera);
    float factor = distance * (float)tan(FOVY / 2) * 2;
    vec3_t delta_x = vec3_mul(left, motion.pan.x * factor);
    vec3_t delta_y = vec3_mul(up, motion.pan.y * factor);
    return vec3_add(delta_x, delta_y);
}

static vec3_t calculate_offset(vec3_t from_target, motion_t motion) {
    float radius = vec3_length(from_target);
    float theta = (float)atan2(from_target.x, from_target.z);  /* azimuth */
    float phi = (float)acos(from_target.y / radius);           /* polar */
    float factor = PI * 2;
    vec3_t offset;

    radius *= (float)pow(0.95, motion.dolly);
    theta -= motion.orbit.x * factor;
    phi -= motion.orbit.y * factor;
    phi = float_clamp(phi, EPSILON, PI - EPSILON);

    offset.x = radius * (float)sin(phi) * (float)sin(theta);
    offset.y = radius * (float)cos(phi);
    offset.z = radius * (float)sin(phi) * (float)cos(theta);

    return offset;
}

void camera_update_transform(camera_t *camera, motion_t motion) {
    vec3_t from_target = vec3_sub(camera->position, camera->target);
    vec3_t from_camera = vec3_sub(camera->target, camera->position);
    vec3_t pan = calculate_pan(from_camera, motion);
    vec3_t offset = calculate_offset(from_target, motion);
    camera->target = vec3_add(camera->target, pan);
    camera->position = vec3_add(camera->target, offset);
}

/* property retrieving */

vec3_t camera_get_position(camera_t *camera) {
    return camera->position;
}

vec3_t camera_get_forward(camera_t *camera) {
    return vec3_normalize(vec3_sub(camera->target, camera->position));
}

mat4_t camera_get_view_matrix(camera_t *camera) {
    return mat4_lookat(camera->position, camera->target, UP);
}

mat4_t camera_get_proj_matrix(camera_t *camera) {
    return mat4_perspective(FOVY, camera->aspect, NEAR, FAR);
}
