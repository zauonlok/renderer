#include <assert.h>
#include <stdlib.h>
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
#define EDGE_SPACE 0.15f
#define EDGE_START (1 - EDGE_SPACE * 0.5f)
#define EDGE_END (EDGE_SPACE * (NUM_EDGES - 0.5f))

typedef struct {scene_t *scene; int layer;} userdata_t;

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

    int row0 = 0;
    int row1 = framebuffer->height - 1;
    int col0 = (int)(start * (framebuffer->width - 1));
    int col1 = (int)(end * (framebuffer->width - 1));
    draw2d_draw_line(framebuffer, color, row0, row1, col0, col1);
}

static texture_t *get_text_texture(int layer) {
    static texture_t **textures = NULL;
    if (textures == NULL) {
        textures = (texture_t**)malloc(sizeof(texture_t*) * NUM_EDGES);
        textures[0] = texture_from_file("common/diffuse.tga");
        textures[1] = texture_from_file("common/specular.tga");
        textures[2] = texture_from_file("common/roughness.tga");
        textures[3] = texture_from_file("common/occlusion.tga");
        textures[4] = texture_from_file("common/normal.tga");
    }
    assert(layer >= 1 && layer <= NUM_EDGES);
    return textures[layer - 1];
}

static void draw_top_text(framebuffer_t *framebuffer, int layer) {
    texture_t *texture = get_text_texture(layer);
    int row = framebuffer->height - texture->height - 10;
    int col = 10;
    draw2d_draw_texture(framebuffer, texture, row, col);
}

static void draw_bottom_text(framebuffer_t *framebuffer, int layer) {
    texture_t *texture = get_text_texture(layer);
    float center = EDGE_START - EDGE_SPACE * ((float)layer - 0.5f);
    int row = 5;
    int col = (int)(center * framebuffer->width - texture->width * 0.5) - 5;
    draw2d_draw_texture(framebuffer, texture, row, col);
}

static void tick_function(context_t *context, void *userdata_) {
    userdata_t *userdata = (userdata_t*)userdata_;
    framebuffer_t *framebuffer = context->framebuffer;
    framedata_t framedata = test_build_framedata(userdata->scene, context);
    userdata->layer = query_curr_layer(context, userdata->layer);
    framedata.layer_view = userdata->layer;
    scene_draw(userdata->scene, framebuffer, &framedata);

    if (userdata->layer == 0) {
        int edge;
        for (edge = 0; edge < NUM_EDGES; edge++) {
            draw_layer_edge(framebuffer, edge);
            draw_bottom_text(framebuffer, edge + 1);
        }
    } else if (userdata->layer > 0) {
        draw_top_text(framebuffer, userdata->layer);
    }
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
