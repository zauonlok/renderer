#ifndef GEOMETRY_H
#define GEOMETRY_H

typedef struct {float x, y;} vec2_t;
typedef struct {float x, y, z;} vec3f_t;
typedef struct {float x, y, z, w;} vec4f_t;
typedef struct {float m[4][4];} mat4f_t;

/* vec2 stuff */
vec2_t vec2_new(float x, float y);
vec2_t vec2_add(vec2_t a, vec2_t b);
vec2_t vec2_sub(vec2_t a, vec2_t b);

/* vec3f stuff */
vec3f_t vec3f_new(float x, float y, float z);
void vec3f_to_array(vec3f_t v, float arr[3]);
vec3f_t vec3f_from_vec4f(vec4f_t v);
vec3f_t vec3f_scale(vec3f_t v, float scale);
vec3f_t vec3f_add(vec3f_t a, vec3f_t b);
vec3f_t vec3f_sub(vec3f_t a, vec3f_t b);
float vec3f_length(vec3f_t v);
vec3f_t vec3f_normalize(vec3f_t v);
float vec3f_dot(vec3f_t a, vec3f_t b);
vec3f_t vec3f_cross(vec3f_t a, vec3f_t b);

/* vec4f stuff */
vec4f_t vec4f_new(float x, float y, float z, float w);
void vec4f_to_array(vec4f_t v, float arr[4]);
vec4f_t vec4f_from_vec3f(vec3f_t v, float w);
vec4f_t vec4f_scale(vec4f_t v, float scale);

/* mat4f stuff */
mat4f_t mat4f_identity();
vec4f_t mat4f_mul_vec4f(mat4f_t m, vec4f_t v);
mat4f_t mat4f_mul_mat4f(mat4f_t a, mat4f_t b);
mat4f_t mat4f_invert_transpose(mat4f_t m);
mat4f_t mat4f_inverse(mat4f_t m);
mat4f_t mat4f_transpose(mat4f_t m);

/*
mat4f_t mat4f_translate(float dx, float dy, float dz);
mat4f_t mat4f_scale(float sx, float sy, float sz);
mat4f_t mat4f_rotate_x(float angle);
mat4f_t mat4f_rotate_y(float angle);
mat4f_t mat4f_rotate_z(float angle);
*/

#endif
