#ifndef GEOMETRY_H
#define GEOMETRY_H

typedef struct {float x, y;} vec2_t;
typedef struct {float x, y, z;} vec3_t;
typedef struct {float x, y, z, w;} vec4_t;
typedef struct {float m[4][4];} mat4_t;

/* vec2 stuff */
vec2_t vec2_new(float x, float y);
vec2_t vec2_add(vec2_t a, vec2_t b);
vec2_t vec2_sub(vec2_t a, vec2_t b);

/* vec3 stuff */
vec3_t vec3_new(float x, float y, float z);
vec3_t vec3_from_vec4(vec4_t v);
void vec3_to_array(vec3_t v, float arr[3]);
vec3_t vec3_add(vec3_t a, vec3_t b);
vec3_t vec3_sub(vec3_t a, vec3_t b);
vec3_t vec3_scale(vec3_t v, float scale);
float vec3_length(vec3_t v);
vec3_t vec3_normalize(vec3_t v);
float vec3_dot(vec3_t a, vec3_t b);
vec3_t vec3_cross(vec3_t a, vec3_t b);

/* vec4 stuff */
vec4_t vec4_new(float x, float y, float z, float w);
vec4_t vec4_from_vec3(vec3_t v, float w);
void vec4_to_array(vec4_t v, float arr[4]);
vec4_t vec4_scale(vec4_t v, float scale);

/* mat4 stuff */
mat4_t mat4_identity();
vec4_t mat4_mul_vec4(mat4_t m, vec4_t v);
mat4_t mat4_mul_mat4(mat4_t a, mat4_t b);
mat4_t mat4_inverse_transpose(mat4_t m);
mat4_t mat4_invert(mat4_t m);
mat4_t mat4_transpose(mat4_t m);

/* common matrices */
mat4_t mat4_translation(float dx, float dy, float dz);
mat4_t mat4_scaling(float sx, float sy, float sz);
mat4_t mat4_rotation(float angle, float vx, float vy, float vz);
mat4_t mat4_rotation_x(float angle);
mat4_t mat4_rotation_y(float angle);
mat4_t mat4_rotation_z(float angle);
mat4_t mat4_ortho(float left, float right, float bottom, float top,
                  float near, float far);
mat4_t mat4_frustum(float left, float right, float bottom, float top,
                    float near, float far);
mat4_t mat4_orthographic(float fovy, float aspect, float near, float far);
mat4_t mat4_perspective(float fovy, float aspect, float near, float far);
mat4_t mat4_camera(vec3_t eye, vec3_t center, vec3_t up);
mat4_t mat4_lookat(vec3_t eye, vec3_t center, vec3_t up);
mat4_t mat4_viewport(int x, int y, int width, int height);

#endif
