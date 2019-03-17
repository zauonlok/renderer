#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/api.h"
#include "test_helper.h"

/* delegate related functions */

static const char *WINDOW_TITLE = "Viewer";
static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 600;

static const vec3_t CAMERA_POSITION = {0, 0, 2};
static const vec3_t CAMERA_TARGET = {0, 0, 0};

static const float LIGHT_THETA = TO_RADIANS(45);
static const float LIGHT_PHI = TO_RADIANS(45);
static const float LIGHT_SPEED = PI;

typedef struct {
    motion_t motion;
    int orbiting, panning;
    vec2_t orbit_pos, pan_pos;
    float light_theta, light_phi;
} record_t;

static vec2_t calculate_delta(vec2_t old_pos, vec2_t new_pos) {
    vec2_t delta = vec2_sub(new_pos, old_pos);
    return vec2_div(delta, (float)WINDOW_HEIGHT);
}

static vec2_t get_cursor_pos(window_t *window) {
    float xpos, ypos;
    input_query_cursor(window, &xpos, &ypos);
    return vec2_new(xpos, ypos);
}

static void button_callback(window_t *window, button_t button, int pressed) {
    record_t *record = (record_t*)window_get_userdata(window);
    motion_t *motion = &record->motion;
    vec2_t cursor_pos = get_cursor_pos(window);
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
    motion_t *motion = &record->motion;
    motion->dolly += offset;
}

static void update_camera(window_t *window, camera_t *camera,
                          record_t *record) {
    motion_t *motion = &record->motion;
    vec2_t cursor_pos = get_cursor_pos(window);
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
            record->light_phi = float_min(record->light_phi + angle, phi_max);
        }
        if (input_key_pressed(window, KEY_W)) {
            float phi_min = EPSILON;
            record->light_phi = float_max(record->light_phi - angle, phi_min);
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

void test_helper(tickfunc_t *tickfunc, void *userdata) {
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

        window_draw_buffer(window, framebuffer);
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

/* scene related functions */

static vec3_t transform_position(vec3_t position, mat4_t transform) {
    vec4_t original = vec4_from_vec3(position, 1);
    vec4_t transformed = mat4_mul_vec4(transform, original);
    return vec3_from_vec4(transformed);
}

static void get_model_bbox(model_t *model, vec3_t *bbmin, vec3_t *bbmax) {
    mesh_t *mesh = model->mesh;
    mat4_t transform = model->transform;
    int num_faces = mesh_get_num_faces(mesh);
    int i, j;

    *bbmin = vec3_new(+1e6, +1e6, +1e6);
    *bbmax = vec3_new(-1e6, -1e6, -1e6);
    for (i = 0; i < num_faces; i++) {
        for (j = 0; j < 3; j++) {
            vertex_t vertex = mesh_get_vertex(mesh, i, j);
            vec3_t position = transform_position(vertex.position, transform);
            *bbmin = vec3_min(*bbmin, position);
            *bbmax = vec3_max(*bbmax, position);
        }
    }
    model->center = vec3_div(vec3_add(*bbmin, *bbmax), 2);
}

static void get_scene_bbox(scene_t *scene, vec3_t *bbmin, vec3_t *bbmax) {
    int num_models = darray_size(scene->models);
    int i;

    *bbmin = vec3_new(+1e6, +1e6, +1e6);
    *bbmax = vec3_new(-1e6, -1e6, -1e6);
    for (i = 0; i < num_models; i++) {
        model_t *model = scene->models[i];
        vec3_t model_bbmin, model_bbmax;
        get_model_bbox(model, &model_bbmin, &model_bbmax);
        *bbmin = vec3_min(*bbmin, model_bbmin);
        *bbmax = vec3_max(*bbmax, model_bbmax);
    }
}

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

scene_t *scene_create(scene_creator_t creators[], const char *scene_name) {
    scene_t *scene = NULL;
    if (scene_name == NULL) {
        int num_creators = 0;
        while (creators[num_creators].scene_name != NULL) {
            num_creators += 1;
        }
        if (num_creators > 0) {
            int index = rand() % num_creators;
            scene_name = creators[index].scene_name;
            scene = creators[index].create();
        }
    } else {
        int i;
        for (i = 0; creators[i].scene_name != NULL; i++) {
            if (strcmp(creators[i].scene_name, scene_name) == 0) {
                scene = creators[i].create();
                break;
            }
        }
    }
    if (scene) {
        vec3_t bbmin, bbmax, center, extend;
        get_scene_bbox(scene, &bbmin, &bbmax);
        center = vec3_div(vec3_add(bbmin, bbmax), 2);
        extend = vec3_sub(bbmax, bbmin);

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

void scene_release(scene_t *scene) {
    int num_models = darray_size(scene->models);
    int i;
    if (scene->skybox) {
        model_t *skybox = scene->skybox;
        skybox->release(skybox);
    }
    for (i = 0; i < num_models; i++) {
        model_t *model = scene->models[i];
        model->release(model);
    }
    darray_free(scene->models);
    free(scene);
}

static int compare_models(const void *model1p, const void *model2p) {
    model_t *model1 = *(model_t**)model1p;
    model_t *model2 = *(model_t**)model2p;

    if (model1->opaque && model2->opaque) {
        return model1->distance < model2->distance ? -1 : 1;
    } else if (model1->opaque && !model2->opaque) {
        return -1;
    } else if (!model1->opaque && model2->opaque) {
        return 1;
    } else {
        return model1->distance < model2->distance ? 1 : -1;
    }
}

void scene_sort_models(scene_t *scene, mat4_t view_matrix) {
    int num_models = darray_size(scene->models);
    int i;
    if (num_models > 1) {
        for (i = 0; i < num_models; i++) {
            model_t *model = scene->models[i];
            vec4_t local_pos = vec4_from_vec3(model->center, 1);
            vec4_t world_pos = mat4_mul_vec4(model->transform, local_pos);
            vec4_t view_pos = mat4_mul_vec4(view_matrix, world_pos);
            model->distance = -view_pos.z;
        }
        qsort(scene->models, num_models, sizeof(model_t*), compare_models);
    }
}

void scene_draw_models(scene_t *scene, framebuffer_t *framebuffer) {
    int num_models = darray_size(scene->models);
    model_t *skybox = scene->skybox;
    int i;

    if (skybox == NULL) {
        for (i = 0; i < num_models; i++) {
            model_t *model = scene->models[i];
            model->draw(model, framebuffer);
        }
    } else {
        int num_opaques = 0;
        for (i = 0; i < num_models; i++) {
            model_t *model = scene->models[i];
            if (model->opaque) {
                num_opaques += 1;
            } else {
                break;
            }
        }

        for (i = 0; i < num_opaques; i++) {
            model_t *model = scene->models[i];
            model->draw(model, framebuffer);
        }
        skybox->draw(skybox, framebuffer);
        for (i = num_opaques; i < num_models; i++) {
            model_t *model = scene->models[i];
            model->draw(model, framebuffer);
        }
    }
}
