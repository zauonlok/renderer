#include <stddef.h>
#include "../core/api.h"
#include "../scenes/pbr_scenes.h"
#include "test_helper.h"
#include "test_pbr.h"

static creator_t g_creators[] = {
    {"assassin", pbr_assassin_scene},
    {"crab", pbr_crab_scene},
    {"dieselpunk", pbr_dieselpunk_scene},
    {"drone", pbr_drone_scene},
    {"helmet", pbr_helmet_scene},
    {"junkrat", pbr_junkrat_scene},
    {"ornitier", pbr_ornitier_scene},
    {"ponycar", pbr_ponycar_scene},
    {"sphere", pbr_sphere_scene},
    {NULL, NULL},
};

static void tick_function(context_t *context, void *userdata) {
    scene_t *scene = (scene_t*)userdata;
    test_draw_scene(scene, context);
}

void test_pbr(int argc, char *argv[]) {
    const char *scene_name = argc > 2 ? argv[2] : NULL;
    scene_t *scene = test_create_scene(g_creators, scene_name);
    if (scene) {
        test_enter_mainloop(tick_function, scene);
        scene_release(scene);
    }
}
