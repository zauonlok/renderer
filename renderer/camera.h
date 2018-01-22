#ifndef CAMERA_H
#define CAMERA_H

#include "geometry.h"
#include "platform.h"

typedef struct camera camera_t;

typedef struct {
    /* camera speed */
    float move_speed;
    float rotate_speed;
    float zoom_speed;
    /* pitch range */
    float pitch_upper;
    float pitch_lower;
    /* zoom range */
    float fovy_default;
    float fovy_minimum;
    /* perspective */
    float aspect;
    float depth_near;
    float depth_far;
} camopt_t;

/* camera creating/releasing */
camera_t *camera_create(vec3_t position, vec3_t forward, float aspect);
void camera_release(camera_t *camera);

/* camera customizing */
camopt_t camera_get_options(camera_t *camera);
void camera_set_options(camera_t *camera, camopt_t options);

/* input processing */
void camera_process_input(camera_t *camera, window_t *window, float delta_time);

/* matrices retrieving */
mat4_t camera_get_view_matrix(camera_t *camera);
mat4_t camera_get_proj_matrix(camera_t *camera);
mat4_t camera_get_viewproj_matrix(camera_t *camera);

#endif
