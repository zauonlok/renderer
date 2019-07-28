#include "../core/api.h"
#include "pbr_scenes.h"
#include "scene_helper.h"

scene_t *pbr_assassin_scene(void) {
    mat4_t translation = mat4_translate(0, -125.815f, 18.898f);
    mat4_t scale = mat4_scale(0.0038f, 0.0038f, 0.0038f);
    mat4_t root = mat4_mul_mat4(scale, translation);
    return helper_load_scene("assassin/assassin.scn", root);
}

scene_t *pbr_buster_scene(void) {
    mat4_t translation = mat4_translate(0, 15.918f, -5.720f);
    mat4_t rotation = mat4_rotate_x(TO_RADIANS(90));
    mat4_t scale = mat4_scale(0.0045f, 0.0045f, 0.0045f);
    mat4_t root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    return helper_load_scene("buster/buster.scn", root);
}

scene_t *pbr_crab_scene(void) {
    mat4_t translation = mat4_translate(-0.285f, 0.780f, 0.572f);
    mat4_t rotation = mat4_rotate_y(TO_RADIANS(180));
    mat4_t scale = mat4_scale(0.167f, 0.167f, 0.167f);
    mat4_t root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    return helper_load_scene("crab/crab.scn", root);
}

scene_t *pbr_dieselpunk_scene(void) {
    mat4_t translation = mat4_translate(1.036f, -114.817f, 27.682f);
    mat4_t rotation = mat4_rotate_y(TO_RADIANS(-90));
    mat4_t scale = mat4_scale(0.0012f, 0.0012f, 0.0012f);
    mat4_t root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    return helper_load_scene("dieselpunk/dieselpunk.scn", root);
}

scene_t *pbr_drone_scene(void) {
    mat4_t translation = mat4_translate(0, -78.288f, -4.447f);
    mat4_t rotation = mat4_rotate_y(TO_RADIANS(180));
    mat4_t scale = mat4_scale(0.0028f, 0.0028f, 0.0028f);
    mat4_t root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    return helper_load_scene("drone/drone.scn", root);
}

scene_t *pbr_helmet_scene(void) {
    mat4_t translation = mat4_translate(0.002f, 0.187f, 0);
    mat4_t rotation = mat4_rotate_x(TO_RADIANS(90));
    mat4_t scale = mat4_scale(0.5f, 0.5f, 0.5f);
    mat4_t root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    return helper_load_scene("helmet/helmet.scn", root);
}

scene_t *pbr_junkrat_scene(void) {
    mat4_t translation = mat4_translate(3.735f, -382.993f, 57.980f);
    mat4_t scale = mat4_scale(0.0013f, 0.0013f, 0.0013f);
    mat4_t root = mat4_mul_mat4(scale, translation);
    return helper_load_scene("junkrat/junkrat.scn", root);
}

scene_t *pbr_ornitier_scene(void) {
    mat4_t translation = mat4_translate(-111.550f, -67.795f, 178.647f);
    mat4_t scale = mat4_scale(0.00095f, 0.00095f, 0.00095f);
    mat4_t root = mat4_mul_mat4(scale, translation);
    return helper_load_scene("ornitier/ornitier.scn", root);
}

scene_t *pbr_ponycar_scene(void) {
    mat4_t translation = mat4_translate(-10.343f, -13.252f, -186.343f);
    mat4_t rotation = mat4_rotate_x(TO_RADIANS(-90));
    mat4_t scale = mat4_scale(0.0015f, 0.0015f, 0.0015f);
    mat4_t root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    return helper_load_scene("ponycar/ponycar.scn", root);
}

scene_t *pbr_sphere_scene(void) {
    mat4_t root = mat4_scale(0.125f, 0.125f, 0.125f);
    return helper_load_scene("common/sphere.scn", root);
}
