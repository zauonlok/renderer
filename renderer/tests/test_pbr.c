#include <stddef.h>
#include "../core/api.h"
#include "../scenes/pbr_scenes.h"
#include "test_helper.h"
#include "test_pbr.h"

static scene_creator_t g_scene_creators[] = {
    {"helmet", pbr_helmet_scene},
    {"helmet2", pbr_helmet2_scene},
    {"ponycar", pbr_ponycar_scene},
    {NULL, NULL},
};

static void tick_function(context_t *context, void *userdata) {
    scene_t *scene = (scene_t*)userdata;
    test_draw_scene(scene, context);
}

void test_pbr(int argc, char *argv[]) {
    const char *scene_name = argc > 2 ? argv[2] : NULL;
    scene_t *scene = test_create_scene(g_scene_creators, scene_name);
    if (scene) {
        test_enter_mainloop(tick_function, scene);
        test_release_scene(scene);
    }
}
