#include <stddef.h>
#include "../core/api.h"
#include "../scenes/blinn_scenes.h"
#include "test_blinn.h"
#include "test_helper.h"

static scene_creator_t g_scene_creators[] = {
    {"centaur", blinn_centaur_scene},
    {"craftsman", blinn_craftsman_scene},
    {"elfgirl", blinn_elfgirl_scene},
    {"kgirls", blinn_kgirls_scene},
    {"mccree", blinn_mccree_scene},
    {"phoenix", blinn_phoenix_scene},
    {"witch", blinn_witch_scene},
    {NULL, NULL},
};

static void tick_function(context_t *context, void *userdata) {
    scene_t *scene = (scene_t*)userdata;
    test_draw_scene(scene, context);
}

void test_blinn(int argc, char *argv[]) {
    const char *scene_name = argc > 2 ? argv[2] : NULL;
    scene_t *scene = test_create_scene(g_scene_creators, scene_name);
    if (scene) {
        test_enter_mainloop(tick_function, scene);
        test_release_scene(scene);
    }
}
