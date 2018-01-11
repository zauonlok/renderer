#ifndef GEOMETRY_H
#define GEOMETRY_H

typedef struct {float x, y;} vec2_t;
typedef struct {float x, y, z;} vec3_t;
typedef struct {float x, y, z, w;} vec4_t;
typedef struct {float m[4][4];} mat4f_t;

/* vec2 stuff */
vec2_t vec2_new(float x, float y);
vec2_t vec2_add(vec2_t a, vec2_t b);
vec2_t vec2_sub(vec2_t a, vec2_t b);

/* vec3 stuff */
vec3_t vec3_new(float x, float y, float z);
void vec3_to_array(vec3_t v, float arr[3]);
vec3_t vec3_from_vec4(vec4_t v);
vec3_t vec3_scale(vec3_t v, float scale);
vec3_t vec3_add(vec3_t a, vec3_t b);
vec3_t vec3_sub(vec3_t a, vec3_t b);
float vec3_length(vec3_t v);
vec3_t vec3_normalize(vec3_t v);
float vec3_dot(vec3_t a, vec3_t b);
vec3_t vec3_cross(vec3_t a, vec3_t b);

/* vec4 stuff */
vec4_t vec4_new(float x, float y, float z, float w);
void vec4_to_array(vec4_t v, float arr[4]);
vec4_t vec4_from_vec3(vec3_t v, float w);
vec4_t vec4_scale(vec4_t v, float scale);

/* mat4f stuff */
mat4f_t mat4f_identity();
vec4_t mat4f_mul_vec4(mat4f_t m, vec4_t v);
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
