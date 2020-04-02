#include <stddef.h>
#include "../core/api.h"
#include "../scenes/blinn_scenes.h"
#include "test_blinn.h"
#include "test_helper.h"

static creator_t g_creators[] = {
    {"azura", blinn_azura_scene},
    {"centaur", blinn_centaur_scene},
    {"craftsman", blinn_craftsman_scene},
    {"elfgirl", blinn_elfgirl_scene},
    {"kgirl", blinn_kgirl_scene},
    {"lighthouse", blinn_lighthouse_scene},
    {"mccree", blinn_mccree_scene},
    {"nier2b", blinn_nier2b_scene},
    {"phoenix", blinn_phoenix_scene},
    {"vivi", blinn_vivi_scene},
    {"whip", blinn_whip_scene},
    {"witch", blinn_witch_scene},
    {NULL, NULL},
};

static void tick_function(context_t *context, void *userdata) {
    scene_t *scene = (scene_t*)userdata;
    perframe_t perframe = test_build_perframe(scene, context);
    test_draw_scene(scene, context->framebuffer, &perframe);
}

void test_blinn(int argc, char *argv[]) {
    const char *scene_name = argc > 2 ? argv[2] : NULL;
    scene_t *scene = test_create_scene(g_creators, scene_name);
    if (scene) {
        test_enter_mainloop(tick_function, scene);
        scene_release(scene);
    }
}
