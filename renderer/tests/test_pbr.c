#include <stddef.h>
#include "../core/api.h"
#include "../scenes/pbr_scenes.h"
#include "test_helper.h"
#include "test_pbr.h"

static creator_t g_creators[] = {
    {"assassin", pbr_assassin_scene},
    {"buster", pbr_buster_scene},
    {"crab", pbr_crab_scene},
    {"dieselpunk", pbr_dieselpunk_scene},
    {"drone", pbr_drone_scene},
    {"helmet", pbr_helmet_scene},
    {"junkrat", pbr_junkrat_scene},
    {"ornitier", pbr_ornitier_scene},
    {"ponycar", pbr_ponycar_scene},
    {"sphere", pbr_sphere_scene},
    {"spheres", pbr_spheres_scene},
    {NULL, NULL},
};

#define NUM_EDGES 5
#define LAYER_WIDTH 0.15f
#define EDGE_START (1 - LAYER_WIDTH * 0.5f)
#define EDGE_END (LAYER_WIDTH * (NUM_EDGES - 0.5f))

typedef struct {scene_t *scene; int layer;} userdata_t;

/*
 * for edge function, see
 * https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/rasterization-stage
 */
static int above_layer_edge(int edge, vec2_t coord) {
    float offset = LAYER_WIDTH * (float)edge;
    vec2_t s = vec2_new(EDGE_START - offset, 0);
    vec2_t e = vec2_new(EDGE_END - offset, 1);
    return (coord.x - s.x) * (e.y - s.y) - (coord.y - s.y) * (e.x - s.x) > 0;
}

static void tick_function(context_t *context, void *userdata_) {
    userdata_t *userdata = (userdata_t*)userdata_;
    framedata_t framedata = test_build_framedata(userdata->scene, context);

    if (context->double_click) {
        if (userdata->layer >= 0) {
            userdata->layer = -1;
        } else {
            userdata->layer = 0;
        }
    }
    if (context->single_click) {
        if (userdata->layer > 0) {
            userdata->layer = 0;
        } else if (userdata->layer == 0) {
            int edge;
            for (edge = 0; edge < NUM_EDGES; edge++) {
                if (above_layer_edge(edge, context->click_pos)) {
                    break;
                }
            }
            if (edge == 0) {
                userdata->layer = -1;
            } else {
                userdata->layer = edge;
            }
        }
    }

    framedata.layer_view = userdata->layer;
    scene_draw(userdata->scene, context->framebuffer, &framedata);
}

void test_pbr(int argc, char *argv[]) {
    const char *scene_name = argc > 2 ? argv[2] : NULL;
    scene_t *scene = test_create_scene(g_creators, scene_name);
    if (scene) {
        userdata_t userdata;
        userdata.scene = scene;
        userdata.layer = -1;
        test_enter_mainloop(tick_function, &userdata);
        scene_release(scene);
    }
}
