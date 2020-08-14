#include <stddef.h>
#include "../core/api.h"
#include "../scenes/pbr_scenes.h"
#include "../shaders/cache_helper.h"
#include "test_helper.h"
#include "test_pbr.h"

static creator_t g_creators[] = {
    {"assassin", pbr_assassin_scene},
    {"buster", pbr_buster_scene},
    {"crab", pbr_crab_scene},
    {"dieselpunk", pbr_dieselpunk_scene},
    {"drone", pbr_drone_scene},
    {"helmet", pbr_helmet_scene},
    {"horse", pbr_horse_scene},
    {"junkrat", pbr_junkrat_scene},
    {"ornitier", pbr_ornitier_scene},
    {"ponycar", pbr_ponycar_scene},
    {"robot", pbr_robot_scene},
    {"sphere", pbr_sphere_scene},
    {"spheres", pbr_spheres_scene},
    {NULL, NULL},
};

#define NUM_EDGES 5
#define EDGE_SPACE 0.15f
#define EDGE_START (1 - EDGE_SPACE * 0.5f)
#define EDGE_END (EDGE_SPACE * (NUM_EDGES - 0.5f))

typedef struct {
    scene_t *scene;
    int layer;
    texture_t *labels[5];
} userdata_t;

static int above_layer_edge(int edge, vec2_t coord) {
    float offset = EDGE_SPACE * (float)edge;
    vec2_t start = vec2_new(EDGE_START - offset, 0);
    vec2_t end = vec2_new(EDGE_END - offset, 1);
    return vec2_edge(start, end, coord) > 0;
}

static int query_curr_layer(context_t *context, int prev_layer) {
    if (context->double_click) {
        if (prev_layer >= 0) {
            return -1;
        } else {
            return 0;
        }
    }
    if (context->single_click) {
        if (prev_layer > 0) {
            return 0;
        } else if (prev_layer == 0) {
            int edge;
            for (edge = 0; edge < NUM_EDGES; edge++) {
                if (above_layer_edge(edge, context->click_pos)) {
                    break;
                }
            }
            if (edge == 0) {
                return -1;
            } else {
                return edge;
            }
        }
    }
    return prev_layer;
}

static void draw_layer_edge(framebuffer_t *framebuffer, int edge) {
    vec4_t color = vec4_new(0.65f, 0.65f, 0.65f, 1);
    float offset = EDGE_SPACE * (float)edge;
    float start = EDGE_START - offset;
    float end = EDGE_END - offset;
    draw2d_draw_line(framebuffer, color, vec2_new(start, 0), vec2_new(end, 1));
}

static void draw_top_text(framebuffer_t *framebuffer, texture_t *labels[5],
                          int layer) {
    texture_t *label = labels[layer - 1];
    float ratio = (float)label->height / (float)framebuffer->height;
    vec2_t origin = vec2_new(0, 1 - ratio);
    draw2d_draw_texture(framebuffer, label, origin);
}

static void draw_bottom_text(framebuffer_t *framebuffer, texture_t *labels[5],
                             int layer) {
    texture_t *label = labels[layer - 1];
    float ratio = (float)label->width / (float)framebuffer->width;
    float center = EDGE_START - EDGE_SPACE * ((float)layer - 0.5f);
    vec2_t origin = vec2_new(center - ratio * 0.5f, 0);
    draw2d_draw_texture(framebuffer, label, origin);
}

static void draw_layer_view(framebuffer_t *framebuffer, userdata_t *userdata) {
    if (userdata->layer == 0) {
        int edge;
        for (edge = 0; edge < NUM_EDGES; edge++) {
            draw_layer_edge(framebuffer, edge);
            draw_bottom_text(framebuffer, userdata->labels, edge + 1);
        }
    } else if (userdata->layer > 0) {
        draw_top_text(framebuffer, userdata->labels, userdata->layer);
    }
}

static void tick_function(context_t *context, void *userdata_) {
    userdata_t *userdata = (userdata_t*)userdata_;
    perframe_t perframe = test_build_perframe(userdata->scene, context);
    userdata->layer = query_curr_layer(context, userdata->layer);
    perframe.layer_view = userdata->layer;
    test_draw_scene(userdata->scene, context->framebuffer, &perframe);
    draw_layer_view(context->framebuffer, userdata);
}

static texture_t *acquire_label_texture(const char *filename) {
    return cache_acquire_texture(filename, USAGE_LDR_COLOR);
}

void test_pbr(int argc, char *argv[]) {
    const char *scene_name = argc > 2 ? argv[2] : NULL;
    scene_t *scene = test_create_scene(g_creators, scene_name);
    if (scene) {
        userdata_t userdata;
        userdata.scene = scene;
        userdata.layer = -1;
        userdata.labels[0] = acquire_label_texture("common/diffuse.tga");
        userdata.labels[1] = acquire_label_texture("common/specular.tga");
        userdata.labels[2] = acquire_label_texture("common/roughness.tga");
        userdata.labels[3] = acquire_label_texture("common/occlusion.tga");
        userdata.labels[4] = acquire_label_texture("common/normal.tga");

        test_enter_mainloop(tick_function, &userdata);
        scene_release(scene);

        cache_release_texture(userdata.labels[0]);
        cache_release_texture(userdata.labels[1]);
        cache_release_texture(userdata.labels[2]);
        cache_release_texture(userdata.labels[3]);
        cache_release_texture(userdata.labels[4]);
    }
}
