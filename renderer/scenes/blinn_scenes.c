#include "../core/api.h"
#include "blinn_scenes.h"
#include "scene_helper.h"

scene_t *blinn_azura_scene(void) {
    mat4_t translation = mat4_translate(-6.073f, -1.278f, 0.280f);
    mat4_t scale = mat4_scale(0.378f, 0.378f, 0.378f);
    mat4_t root = mat4_mul_mat4(scale, translation);
    return scene_from_file("azura/azura.scn", root);
}

scene_t *blinn_centaur_scene(void) {
    mat4_t translation = mat4_translate(0.154f, -7.579f, -30.749f);
    mat4_t rotation_x = mat4_rotate_x(TO_RADIANS(-90));
    mat4_t rotation_y = mat4_rotate_y(TO_RADIANS(-90));
    mat4_t rotation = mat4_mul_mat4(rotation_y, rotation_x);
    mat4_t scale = mat4_scale(0.016f, 0.016f, 0.016f);
    mat4_t root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    return scene_from_file("centaur/centaur.scn", root);
}

scene_t *blinn_craftsman_scene(void) {
    mat4_t translation = mat4_translate(-1.668f, -27.061f, -10.834f);
    mat4_t scale = mat4_scale(0.016f, 0.016f, 0.016f);
    mat4_t root = mat4_mul_mat4(scale, translation);
    return scene_from_file("craftsman/craftsman.scn", root);
}

scene_t *blinn_elfgirl_scene(void) {
    mat4_t translation = mat4_translate(2.449f, -2.472f, -20.907f);
    mat4_t rotation = mat4_rotate_x(TO_RADIANS(-90));
    mat4_t scale = mat4_scale(0.023f, 0.023f, 0.023f);
    mat4_t root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    return scene_from_file("elfgirl/elfgirl.scn", root);
}

scene_t *blinn_kgirl_scene(void) {
    mat4_t translation = mat4_translate(0, -4.937f, -96.547f);
    mat4_t rotation_x = mat4_rotate_x(TO_RADIANS(-90));
    mat4_t rotation_y = mat4_rotate_y(TO_RADIANS(90));
    mat4_t rotation = mat4_mul_mat4(rotation_y, rotation_x);
    mat4_t scale = mat4_scale(0.005f, 0.005f, 0.005f);
    mat4_t root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    return scene_from_file("kgirl/kgirl.scn", root);
}

scene_t *blinn_lighthouse_scene(void) {
    mat4_t translation = mat4_translate(-78.203f, -222.929f, 16.181f);
    mat4_t rotation = mat4_rotate_y(TO_RADIANS(-135));
    mat4_t scale = mat4_scale(0.0016f, 0.0016f, 0.0016f);
    mat4_t root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    return scene_from_file("lighthouse/lighthouse.scn", root);
}

scene_t *blinn_mccree_scene(void) {
    mat4_t translation = mat4_translate(0.108f, -1.479f, 0.034f);
    mat4_t scale = mat4_scale(0.337f, 0.337f, 0.337f);
    mat4_t root = mat4_mul_mat4(scale, translation);
    return scene_from_file("mccree/mccree.scn", root);
}

scene_t *blinn_nier2b_scene(void) {
    mat4_t translation = mat4_translate(4.785f, -105.275f, -23.067f);
    mat4_t rotation = mat4_rotate_y(TO_RADIANS(90));
    mat4_t scale = mat4_scale(0.004f, 0.004f, 0.004f);
    mat4_t root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    return scene_from_file("nier2b/nier2b.scn", root);
}

scene_t *blinn_phoenix_scene(void) {
    mat4_t translation = mat4_translate(376.905f, -169.495f, 0);
    mat4_t rotation = mat4_rotate_y(TO_RADIANS(180));
    mat4_t scale = mat4_scale(0.001f, 0.001f, 0.001f);
    mat4_t root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    return scene_from_file("phoenix/phoenix.scn", root);
}

scene_t *blinn_vivi_scene(void) {
    mat4_t translation = mat4_translate(0, 0, -1.369f);
    mat4_t rotation = mat4_rotate_x(TO_RADIANS(-90));
    mat4_t scale = mat4_scale(0.331f, 0.331f, 0.331f);
    mat4_t root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    return scene_from_file("vivi/vivi.scn", root);
}

scene_t *blinn_whip_scene(void) {
    mat4_t translation = mat4_translate(-3732.619f, -93.643f, -1561.663f);
    mat4_t rotation = mat4_rotate_x(TO_RADIANS(90));
    mat4_t scale = mat4_scale(0.0004f, 0.0004f, 0.0004f);
    mat4_t root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    return scene_from_file("whip/whip.scn", root);
}

scene_t *blinn_witch_scene(void) {
    mat4_t translation = mat4_translate(-17.924f, -16.974f, -32.691f);
    mat4_t rotation = mat4_rotate_x(TO_RADIANS(-90));
    mat4_t scale = mat4_scale(0.02f, 0.02f, 0.02f);
    mat4_t root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    return scene_from_file("witch/witch.scn", root);
}
