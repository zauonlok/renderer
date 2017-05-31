#include "geometry.h"

vec2i_t vec2i_new(int x, int y) {
    vec2i_t v;
    v.x = x;
    v.y = y;
    return v;
}

vec3i_t vec3i_new(int x, int y, int z) {
    vec3i_t v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

vec2f_t vec2f_new(float x, float y) {
    vec2f_t v;
    v.x = x;
    v.y = y;
    return v;
}

vec3f_t vec3f_new(float x, float y, float z) {
    vec3f_t v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

vec2i_t vec2i_add(vec2i_t v0, vec2i_t v1) {
    vec2i_t v;
    v.x = v0.x + v1.x;
    v.y = v0.y + v1.y;
    return v;
}

vec2i_t vec2i_sub(vec2i_t v0, vec2i_t v1) {
    vec2i_t v;
    v.x = v0.x - v1.x;
    v.y = v0.y - v1.y;
    return v;
}

