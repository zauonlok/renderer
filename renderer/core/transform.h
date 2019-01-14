#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "geometry.h"

mat4_t mat4_translate(float tx, float ty, float tz);
mat4_t mat4_scale(float sx, float sy, float sz);
mat4_t mat4_rotate(float angle, float vx, float vy, float vz);
mat4_t mat4_rotate_x(float angle);
mat4_t mat4_rotate_y(float angle);
mat4_t mat4_rotate_z(float angle);

mat4_t mat4_lookat(vec3_t eye, vec3_t target, vec3_t up);
mat4_t mat4_ortho(float left, float right, float bottom, float top,
                  float near, float far);
mat4_t mat4_frustum(float left, float right, float bottom, float top,
                    float near, float far);
mat4_t mat4_orthographic(float right, float top, float near, float far);
mat4_t mat4_perspective(float fovy, float aspect, float near, float far);

#endif
