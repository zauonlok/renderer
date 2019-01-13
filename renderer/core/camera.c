#include "camera.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "geometry.h"

static const float SPEED = 0.001f;
static const float INERTIA = 0.9f;
static const vec3_t WORLD_UP = {0, 1, 0};

static const float NEAR = 0.1f;
static const float FAR = 1000;
static const float FOVY = TO_RADIANS(45);

struct camera {
    vec3_t position;
    vec3_t target;
    float aspect;
    /* delta data */
    float delta_theta;
    float delta_phi;
    float delta_dolly;
    vec2_t delta_pan;
};

/* camera creating/releasing/updating */

camera_t *camera_create(vec3_t position, vec3_t target, float aspect) {
    camera_t *camera;

    assert(vec3_length(vec3_sub(position, target)) > EPSILON && aspect > 0);

    camera = (camera_t*)malloc(sizeof(camera_t));
    memset(camera, 0, sizeof(camera_t));
    camera->position = position;
    camera->target   = target;
    camera->aspect   = aspect;

    return camera;
}

void camera_release(camera_t *camera) {
    free(camera);
}

static vec3_t calculate_pan(camera_t *camera) {
    vec3_t position = camera->position;
    vec3_t target = camera->target;

    vec3_t forward = vec3_normalize(vec3_sub(target, position));
    vec3_t left = vec3_cross(WORLD_UP, forward);
    vec3_t up = vec3_cross(forward, left);

    vec3_t delta_x = vec3_mul(left, camera->delta_pan.x * SPEED);
    vec3_t delta_y = vec3_mul(up, camera->delta_pan.y * SPEED);
    return vec3_add(delta_x, delta_y);
}

static double clamp_double(double value, double min, double max) {
    assert(min <= max);
    return (value < min) ? min : ((value > max) ? max : value);
}

static vec3_t calculate_offset(camera_t *camera) {
    vec3_t offset = vec3_sub(camera->position, camera->target);
    double radius = vec3_length(offset);       /* distance */
    double theta = atan2(offset.x, offset.z);  /* azimuth angle */
    double phi = acos(offset.y / radius);      /* polar angle */

    radius *= pow(0.95, camera->delta_dolly);
    theta += camera->delta_theta * SPEED;
    phi += camera->delta_phi * SPEED;
    phi = clamp_double(phi, EPSILON, PI - EPSILON);

    offset = vec3_new(
        (float)(radius * sin(phi) * sin(theta)),
        (float)(radius * cos(phi)),
        (float)(radius * sin(phi) * cos(theta))
    );
    return offset;
}

void camera_orbit_update(camera_t *camera, motion_t motion) {
    vec3_t pan;
    vec3_t offset;

    camera->delta_theta -= motion.orbit.x;
    camera->delta_phi -= motion.orbit.y;
    camera->delta_dolly += motion.dolly;
    camera->delta_pan = vec2_add(camera->delta_pan, motion.pan);

    pan = calculate_pan(camera);
    offset = calculate_offset(camera);
    camera->target = vec3_add(camera->target, pan);
    camera->position = vec3_add(camera->target, offset);

    camera->delta_theta *= INERTIA;
    camera->delta_phi *= INERTIA;
    camera->delta_dolly *= INERTIA;
    camera->delta_pan = vec2_mul(camera->delta_pan, INERTIA);
}

/* propety retrieving */

vec3_t camera_get_position(camera_t *camera) {
    return camera->position;
}

vec3_t camera_get_forward(camera_t *camera) {
    return vec3_normalize(vec3_sub(camera->target, camera->position));
}

mat4_t camera_get_view_matrix(camera_t *camera) {
    return mat4_lookat(camera->position, camera->target, WORLD_UP);
}

mat4_t camera_get_proj_matrix(camera_t *camera) {
    return mat4_perspective(FOVY, camera->aspect, NEAR, FAR);
}
