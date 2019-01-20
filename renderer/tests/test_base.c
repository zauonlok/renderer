#include "test_base.h"
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../core/apis.h"

/* test delegate functions */

static const char *WINDOW_TITLE = "Viewer";
static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 600;

static const vec3_t CAMERA_POSITION = {0, 0, 2};
static const vec3_t CAMERA_TARGET = {0, 0, 0};

static const float LIGHT_THETA = TO_RADIANS(45);
static const float LIGHT_PHI = TO_RADIANS(45);
static const float LIGHT_SPEED = PI;

typedef struct {
    motion_t next_motion;
    int orbiting, panning;
    vec2_t orbit_pos, pan_pos;
    float light_theta, light_phi;
} record_t;

static vec2_t calculate_delta(vec2_t old_pos, vec2_t new_pos) {
    vec2_t delta = vec2_sub(new_pos, old_pos);
    return vec2_div(delta, (float)WINDOW_HEIGHT);
}

static void button_callback(window_t *window, button_t button, int pressed) {
    record_t *record = (record_t*)window_get_userdata(window);
    motion_t *motion = &record->next_motion;
    vec2_t cursor_pos = input_query_cursor(window);
    if (button == BUTTON_L) {
        if (pressed) {
            record->orbiting = 1;
            record->orbit_pos = cursor_pos;
        } else {
            vec2_t delta = calculate_delta(record->orbit_pos, cursor_pos);
            record->orbiting = 0;
            motion->orbit = vec2_add(motion->orbit, delta);
        }
    } else if (button == BUTTON_R) {
        if (pressed) {
            record->panning = 1;
            record->pan_pos = cursor_pos;
        } else {
            vec2_t delta = calculate_delta(record->pan_pos, cursor_pos);
            record->panning = 0;
            motion->pan = vec2_add(motion->pan, delta);
        }
    }
}

static void scroll_callback(window_t *window, float offset) {
    record_t *record = (record_t*)window_get_userdata(window);
    motion_t *motion = &record->next_motion;
    motion->dolly += offset;
}

static void update_camera(window_t *window, camera_t *camera,
                          record_t *record) {
    motion_t *motion = &record->next_motion;
    vec2_t cursor_pos = input_query_cursor(window);
    if (record->orbiting) {
        vec2_t delta = calculate_delta(record->orbit_pos, cursor_pos);
        motion->orbit = vec2_add(motion->orbit, delta);
        record->orbit_pos = cursor_pos;
    }
    if (record->panning) {
        vec2_t delta = calculate_delta(record->pan_pos, cursor_pos);
        motion->pan = vec2_add(motion->pan, delta);
        record->pan_pos = cursor_pos;
    }
    if (input_key_pressed(window, KEY_SPACE)) {
        camera_set_transform(camera, CAMERA_POSITION, CAMERA_TARGET);
    } else {
        camera_orbit_update(camera, *motion);
    }
    memset(motion, 0, sizeof(motion_t));
}

static void update_light(window_t *window, float delta_time,
                         record_t *record) {
    if (input_key_pressed(window, KEY_SPACE)) {
        record->light_theta = LIGHT_THETA;
        record->light_phi = LIGHT_PHI;
    } else {
        float angle = LIGHT_SPEED * delta_time;
        if (input_key_pressed(window, KEY_A)) {
            record->light_theta -= angle;
        }
        if (input_key_pressed(window, KEY_D)) {
            record->light_theta += angle;
        }
        if (input_key_pressed(window, KEY_S)) {
            float phi_max = PI - EPSILON;
            record->light_phi += angle;
            if (record->light_phi > phi_max) {
                record->light_phi = phi_max;
            }
        }
        if (input_key_pressed(window, KEY_W)) {
            float phi_min = EPSILON;
            record->light_phi -= angle;
            if (record->light_phi < phi_min) {
                record->light_phi = phi_min;
            }
        }
    }
}

static vec3_t calculate_light(record_t *record) {
    float theta = record->light_theta;
    float phi = record->light_phi;
    float x = (float)sin(phi) * (float)sin(theta);
    float y = (float)cos(phi);
    float z = (float)sin(phi) * (float)cos(theta);
    return vec3_new(-x, -y, -z);
}

void test_base(tickfunc_t *tickfunc, void *userdata) {
    window_t *window;
    framebuffer_t *framebuffer;
    camera_t *camera;
    record_t record;
    callbacks_t callbacks;
    context_t context;
    float aspect;
    float prev_time;
    float report_time;
    int num_frames;

    window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
    framebuffer = framebuffer_create(WINDOW_WIDTH, WINDOW_HEIGHT);
    aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
    camera = camera_create(CAMERA_POSITION, CAMERA_TARGET, aspect);

    memset(&record, 0, sizeof(record_t));
    record.light_theta = LIGHT_THETA;
    record.light_phi   = LIGHT_PHI;

    memset(&callbacks, 0, sizeof(callbacks_t));
    callbacks.button_callback = button_callback;
    callbacks.scroll_callback = scroll_callback;

    memset(&context, 0, sizeof(context_t));
    context.framebuffer = framebuffer;
    context.camera      = camera;

    window_set_userdata(window, &record);
    input_set_callbacks(window, callbacks);

    num_frames = 0;
    prev_time = input_get_time();
    report_time = prev_time;
    while (!window_should_close(window)) {
        float curr_time = input_get_time();
        float delta_time = curr_time - prev_time;
        prev_time = curr_time;

        update_camera(window, camera, &record);
        update_light(window, delta_time, &record);

        context.light_dir = calculate_light(&record);
        context.delta_time = delta_time;
        tickfunc(&context, userdata);

        window_draw_buffer(window, framebuffer->colorbuffer);
        num_frames += 1;
        if (curr_time - report_time >= 1) {
            printf("fps: %d\n", num_frames);
            num_frames = 0;
            report_time = curr_time;
        }

        input_poll_events();
    }

    window_destroy(window);
    framebuffer_release(framebuffer);
    camera_release(camera);
}

/* scene helper functions */

static int count_num_faces(scene_t *scene) {
    int num_models = darray_size(scene->models);
    int num_faces = 0;
    int i;
    for (i = 0; i < num_models; i++) {
        model_t *model = scene->models[i];
        num_faces += mesh_get_num_faces(model->mesh);
    }
    return num_faces;
}

static float min_float(float a, float b) {
    return a < b ? a : b;
}

static float max_float(float a, float b) {
    return a > b ? a : b;
}

static void calculate_bbox(scene_t *scene,
                           vec3_t *out_bbmin, vec3_t *out_bbmax,
                           vec3_t *out_center, vec3_t *out_extend) {
    int num_models = darray_size(scene->models);
    vec3_t bbmin = vec3_new(+1e6, +1e6, +1e6);
    vec3_t bbmax = vec3_new(-1e6, -1e6, -1e6);
    int i, j, k;
    for (i = 0; i < num_models; i++) {
        mesh_t *mesh = scene->models[i]->mesh;
        int num_faces = mesh_get_num_faces(mesh);
        for (j = 0; j < num_faces; j++) {
            for (k = 0; k < 3; k++) {
                vec3_t position = mesh_get_position(mesh, j, k);
                bbmin.x = min_float(bbmin.x, position.x);
                bbmin.y = min_float(bbmin.y, position.y);
                bbmin.z = min_float(bbmin.z, position.z);
                bbmax.x = max_float(bbmax.x, position.x);
                bbmax.y = max_float(bbmax.y, position.y);
                bbmax.z = max_float(bbmax.z, position.z);
            }
        }
    }
    *out_bbmin = bbmin;
    *out_bbmax = bbmax;
    *out_center = vec3_div(vec3_add(bbmin, bbmax), 2);
    *out_extend = vec3_sub(bbmax, bbmin);
}

scene_t *scene_create(scene_entry_t scene_entries[], int num_entries,
                      const char *scene_name) {
    scene_t *scene = NULL;
    assert(num_entries > 0);
    if (scene_name == NULL) {
        int index = rand() % num_entries;
        scene_name = scene_entries[index].scene_name;
        scene = scene_entries[index].scene_ctor();
    } else {
        int i;
        for (i = 0; i < num_entries; i++) {
            if (strcmp(scene_name, scene_entries[i].scene_name) == 0) {
                scene = scene_entries[i].scene_ctor();
                break;
            }
        }
    }
    if (scene) {
        vec3_t bbmin, bbmax, center, extend;
        calculate_bbox(scene, &bbmin, &bbmax, &center, &extend);
        printf("scene: %s\n", scene_name);
        printf("faces: %d\n", count_num_faces(scene));
        printf("bbmin: [%.3f, %.3f, %.3f]\n", bbmin.x, bbmin.y, bbmin.z);
        printf("bbmax: [%.3f, %.3f, %.3f]\n", bbmax.x, bbmax.y, bbmax.z);
        printf("center: [%.3f, %.3f, %.3f]\n", center.x, center.y, center.z);
        printf("extend: [%.3f, %.3f, %.3f]\n", extend.x, extend.y, extend.z);
    } else {
        printf("scene not found: %s\n", scene_name);
    }
    return scene;
}

void scene_release(scene_t *scene, void (*model_dtor)(model_t *model)) {
    int num_models = darray_size(scene->models);
    int i;
    for (i = 0; i < num_models; i++) {
        model_dtor(scene->models[i]);
    }
    darray_free(scene->models);
    free(scene);
}
