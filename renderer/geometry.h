#ifndef GEOMETRY_H
#define GEOMETRY_H

typedef struct {float x, y;} vec2f_t;
typedef struct {float x, y, z;} vec3f_t;
typedef struct {float x, y, z, w;} vec4f_t;
typedef struct {float m[4][4];} mat4f_t;

vec2f_t vec2f_new(float x, float y);
vec3f_t vec3f_new(float x, float y, float z);
vec4f_t vec4f_new(float x, float y, float z, float w);
mat4f_t mat4f_identity();

vec3f_t vec3f_add(vec3f_t a, vec3f_t b);
vec3f_t vec3f_sub(vec3f_t a, vec3f_t b);
float vec3f_length(vec3f_t v);
vec3f_t vec3f_normalize(vec3f_t v);
float vec3f_dot(vec3f_t a, vec3f_t b);
vec3f_t vec3f_cross(vec3f_t a, vec3f_t b);

vec4f_t mat4f_mul_vec4f(mat4f_t m, vec4f_t v);
mat4f_t mat4f_mul_mat4f(mat4f_t a, mat4f_t b);

void vec3f_to_array(vec3f_t v, float arr[3]) {
    arr[0] = v.x;
    arr[1] = v.y;
    arr[2] = v.z;
}


vec2f_t vec2f_add(vec2f_t a, vec2f_t b) {
    return vec2f_new(a.x + b.x, a.y + b.y);
}


vec2f_t vec2f_sub(vec2f_t a, vec2f_t b) {
    return vec2f_new(a.x - b.x, a.y - b.y);
}


/*

mat4f_t mat4f_translate(float dx, float dy, float dz);
mat4f_t mat4f_scale(float sx, float sy, float sz);
mat4f_t mat4f_rotate_x(float angle);
mat4f_t mat4f_rotate_y(float angle);
mat4f_t mat4f_rotate_z(float angle);

*/



#endif
