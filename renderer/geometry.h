#ifndef GEOMETRY_H
#define GEOMETRY_H

typedef struct {int x, y;} vec2i_t;
typedef struct {int x, y, z;} vec3i_t;

typedef struct {float x, y;} vec2f_t;
typedef struct {float x, y, z;} vec3f_t;
typedef struct {float x, y, z, w;} vec4f_t;

typedef struct {float m[4][4];} mat4f_t;

vec2i_t vec2i_new(int x, int y);
vec3i_t vec3i_new(int x, int y, int z);

vec2f_t vec2f_new(float x, float y);
vec3f_t vec3f_new(float x, float y, float z);
vec4f_t vec4f_new(float x, float y, float z, float w);

mat4f_t mat4f_identity();
mat4f_t mat4f_translate(float dx, float dy, float dz);
mat4f_t mat4f_scale(float sx, float sy, float sz);
mat4f_t mat4f_rotate_x(float angle);
mat4f_t mat4f_rotate_y(float angle);
mat4f_t mat4f_rotate_z(float angle);

vec2i_t vec2i_add(vec2i_t v0, vec2i_t v1);
vec2i_t vec2i_sub(vec2i_t v0, vec2i_t v1);

vec3f_t vec3f_normalize(vec3f_t v);
vec3f_t vec3f_sub(vec3f_t a, vec3f_t b);
vec3f_t vec3f_cross(vec3f_t a, vec3f_t b);
float vec3f_dot(vec3f_t a, vec3f_t b);

mat4f_t mat4f_mul_mat4f(mat4f_t a, mat4f_t b);

#endif
