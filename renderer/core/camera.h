#ifndef CAMERA_H
#define CAMERA_H

#include "maths.h"

typedef struct camera camera_t;
typedef struct {vec2_t orbit; vec2_t pan; float dolly;} motion_t;

/* camera creating/releasing */
camera_t *camera_create(vec3_t position, vec3_t target, float aspect);
void camera_release(camera_t *camera);

/* camera updating */
void camera_set_transform(camera_t *camera, vec3_t position, vec3_t target);
void camera_update_transform(camera_t *camera, motion_t motion);

/* property retrieving */
vec3_t camera_get_position(camera_t *camera);
vec3_t camera_get_forward(camera_t *camera);
mat4_t camera_get_view_matrix(camera_t *camera);
mat4_t camera_get_proj_matrix(camera_t *camera);

#endif
