#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/api.h"
#include "test_helper.h"

/* mainloop related functions */

static const char *const WINDOW_TITLE = "Viewer";
static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 600;

static const vec3_t CAMERA_POSITION = {0, 0, 1.5f};
static const vec3_t CAMERA_TARGET = {0, 0, 0};

static const float LIGHT_THETA = TO_RADIANS(45);
static const float LIGHT_PHI = TO_RADIANS(45);
static const float LIGHT_SPEED = PI;

static const float CLICK_DELAY = 0.25f;

typedef struct {
    /* orbit */
    int is_orbiting;
    vec2_t orbit_pos;
    vec2_t orbit_delta;
    /* pan */
    int is_panning;
    vec2_t pan_pos;
    vec2_t pan_delta;
    /* zoom */
    float dolly_delta;
    /* light */
    float light_theta;
    float light_phi;
    /* click */
    float press_time;
    float release_time;
    vec2_t press_pos;
    vec2_t release_pos;
    int single_click;
    int double_click;
    vec2_t click_pos;
} record_t;

static vec2_t get_pos_delta(vec2_t old_pos, vec2_t new_pos) {
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
    vec2_t cursor_pos = get_cursor_pos(window);
    if (button == BUTTON_L) {
        float curr_time = platform_get_time();
        if (pressed) {
            record->is_orbiting = 1;
            record->orbit_pos = cursor_pos;
            record->press_time = curr_time;
            record->press_pos = cursor_pos;
        } else {
            float prev_time = record->release_time;
            vec2_t pos_delta = get_pos_delta(record->orbit_pos, cursor_pos);
            record->is_orbiting = 0;
            record->orbit_delta = vec2_add(record->orbit_delta, pos_delta);
            if (prev_time && curr_time - prev_time < CLICK_DELAY) {
                record->double_click = 1;
                record->release_time = 0;
            } else {
                record->release_time = curr_time;
                record->release_pos = cursor_pos;
            }
        }
    } else if (button == BUTTON_R) {
        if (pressed) {
            record->is_panning = 1;
            record->pan_pos = cursor_pos;
        } else {
            vec2_t pos_delta = get_pos_delta(record->pan_pos, cursor_pos);
            record->is_panning = 0;
            record->pan_delta = vec2_add(record->pan_delta, pos_delta);
        }
    }
}

static void scroll_callback(window_t *window, float offset) {
    record_t *record = (record_t*)window_get_userdata(window);
    record->dolly_delta += offset;
}

static void update_camera(window_t *window, camera_t *camera,
                          record_t *record) {
    vec2_t cursor_pos = get_cursor_pos(window);
    if (record->is_orbiting) {
        vec2_t pos_delta = get_pos_delta(record->orbit_pos, cursor_pos);
        record->orbit_delta = vec2_add(record->orbit_delta, pos_delta);
        record->orbit_pos = cursor_pos;
    }
    if (record->is_panning) {
        vec2_t pos_delta = get_pos_delta(record->pan_pos, cursor_pos);
        record->pan_delta = vec2_add(record->pan_delta, pos_delta);
        record->pan_pos = cursor_pos;
    }
    if (input_key_pressed(window, KEY_SPACE)) {
        camera_set_transform(camera, CAMERA_POSITION, CAMERA_TARGET);
    } else {
        motion_t motion;
        motion.orbit = record->orbit_delta;
        motion.pan = record->pan_delta;
        motion.dolly = record->dolly_delta;
        camera_update_transform(camera, motion);
    }
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

static void update_click(float curr_time, record_t *record) {
    float last_time = record->release_time;
    if (last_time && curr_time - last_time > CLICK_DELAY) {
        vec2_t pos_delta = vec2_sub(record->release_pos, record->press_pos);
        if (vec2_length(pos_delta) < 5) {
            record->single_click = 1;
        }
        record->release_time = 0;
    }
    if (record->single_click || record->double_click) {
        float click_x = record->release_pos.x / WINDOW_WIDTH;
        float click_y = record->release_pos.y / WINDOW_HEIGHT;
        record->click_pos = vec2_new(click_x, 1 - click_y);
    }
}

static vec3_t get_light_dir(record_t *record) {
    float theta = record->light_theta;
    float phi = record->light_phi;
    float x = (float)sin(phi) * (float)sin(theta);
    float y = (float)cos(phi);
    float z = (float)sin(phi) * (float)cos(theta);
    return vec3_new(-x, -y, -z);
}

void test_enter_mainloop(tickfunc_t *tickfunc, void *userdata) {
    window_t *window;
    framebuffer_t *framebuffer;
    camera_t *camera;
    record_t record;
    callbacks_t callbacks;
    context_t context;
    float aspect;
    float prev_time;
    float print_time;
    int num_frames;

    window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
    framebuffer = framebuffer_create(WINDOW_WIDTH, WINDOW_HEIGHT);
    aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
    camera = camera_create(CAMERA_POSITION, CAMERA_TARGET, aspect);

    memset(&record, 0, sizeof(record_t));
    record.light_theta = LIGHT_THETA;
    record.light_phi = LIGHT_PHI;

    memset(&callbacks, 0, sizeof(callbacks_t));
    callbacks.button_callback = button_callback;
    callbacks.scroll_callback = scroll_callback;

    memset(&context, 0, sizeof(context_t));
    context.framebuffer = framebuffer;
    context.camera = camera;

    window_set_userdata(window, &record);
    input_set_callbacks(window, callbacks);

    num_frames = 0;
    prev_time = platform_get_time();
    print_time = prev_time;
    while (!window_should_close(window)) {
        float curr_time = platform_get_time();
        float delta_time = curr_time - prev_time;

        update_camera(window, camera, &record);
        update_light(window, delta_time, &record);
        update_click(curr_time, &record);

        context.light_dir = get_light_dir(&record);
        context.click_pos = record.click_pos;
        context.single_click = record.single_click;
        context.double_click = record.double_click;
        context.frame_time = curr_time;
        context.delta_time = delta_time;
        tickfunc(&context, userdata);

        window_draw_buffer(window, framebuffer);
        num_frames += 1;
        if (curr_time - print_time >= 1) {
            int sum_millis = (int)((curr_time - print_time) * 1000);
            int avg_millis = sum_millis / num_frames;
            printf("fps: %3d, avg: %3d ms\n", num_frames, avg_millis);
            num_frames = 0;
            print_time = curr_time;
        }
        prev_time = curr_time;

        record.orbit_delta = vec2_new(0, 0);
        record.pan_delta = vec2_new(0, 0);
        record.dolly_delta = 0;
        record.single_click = 0;
        record.double_click = 0;

        input_poll_events();
    }

    window_destroy(window);
    framebuffer_release(framebuffer);
    camera_release(camera);
}

/* scene related functions */

typedef struct {vec3_t min; vec3_t max;} bbox_t;

static bbox_t get_model_bbox(model_t *model) {
    mesh_t *mesh = model->mesh;
    int num_faces = mesh_get_num_faces(mesh);
    vertex_t *vertices = mesh_get_vertices(mesh);
    mat4_t model_matrix = model->transform;
    bbox_t bbox;
    int i, j;

    if (model->skeleton && model->attached >= 0) {
        mat4_t *joint_matrices;
        mat4_t node_matrix;
        skeleton_update_joints(model->skeleton, 0);
        joint_matrices = skeleton_get_joint_matrices(model->skeleton);
        node_matrix = joint_matrices[model->attached];
        model_matrix = mat4_mul_mat4(model_matrix, node_matrix);
    }

    bbox.min = vec3_new(+1e6, +1e6, +1e6);
    bbox.max = vec3_new(-1e6, -1e6, -1e6);
    for (i = 0; i < num_faces; i++) {
        for (j = 0; j < 3; j++) {
            vertex_t vertex = vertices[i * 3 + j];
            vec4_t local_pos = vec4_from_vec3(vertex.position, 1);
            vec4_t world_pos = mat4_mul_vec4(model_matrix, local_pos);
            bbox.min = vec3_min(bbox.min, vec3_from_vec4(world_pos));
            bbox.max = vec3_max(bbox.max, vec3_from_vec4(world_pos));
        }
    }
    return bbox;
}

static bbox_t get_scene_bbox(scene_t *scene) {
    int num_models = darray_size(scene->models);
    bbox_t bbox;
    int i;

    bbox.min = vec3_new(+1e6, +1e6, +1e6);
    bbox.max = vec3_new(-1e6, -1e6, -1e6);
    for (i = 0; i < num_models; i++) {
        model_t *model = scene->models[i];
        bbox_t model_bbox = get_model_bbox(model);
        bbox.min = vec3_min(bbox.min, model_bbox.min);
        bbox.max = vec3_max(bbox.max, model_bbox.max);
    }
    return bbox;
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

scene_t *test_create_scene(creator_t creators[], const char *scene_name) {
    scene_t *scene = NULL;
    if (scene_name == NULL) {
        int num_creators = 0;
        while (creators[num_creators].scene_name != NULL) {
            num_creators += 1;
        }
        if (num_creators > 0) {
            int index = rand() % num_creators;
            scene_name = creators[index].scene_name;
            printf("scene: %s\n", scene_name);
            scene = creators[index].create_scene();
        }
    } else {
        int i;
        for (i = 0; creators[i].scene_name != NULL; i++) {
            if (strcmp(creators[i].scene_name, scene_name) == 0) {
                printf("scene: %s\n", scene_name);
                scene = creators[i].create_scene();
                break;
            }
        }
    }
    if (scene) {
        int num_faces = count_num_faces(scene);
        bbox_t bbox = get_scene_bbox(scene);
        vec3_t center = vec3_div(vec3_add(bbox.min, bbox.max), 2);
        vec3_t extent = vec3_sub(bbox.max, bbox.min);
        int with_skybox = scene->skybox != NULL;
        int with_shadow = scene->shadow_map != NULL;
        int with_ambient = scene->ambient_intensity > 0;
        int with_punctual = scene->punctual_intensity > 0;

        printf("faces: %d\n", num_faces);
        printf("center: [%.3f, %.3f, %.3f]\n", center.x, center.y, center.z);
        printf("extent: [%.3f, %.3f, %.3f]\n", extent.x, extent.y, extent.z);
        printf("skybox: %s\n", with_skybox ? "on" : "off");
        printf("shadow: %s\n", with_shadow ? "on" : "off");
        printf("ambient: %s\n", with_ambient ? "on" : "off");
        printf("punctual: %s\n", with_punctual ? "on" : "off");
    } else {
        int i;
        printf("scene not found: %s\n", scene_name);
        printf("available scenes: ");
        for (i = 0; creators[i].scene_name != NULL; i++) {
            if (creators[i + 1].scene_name != NULL) {
                printf("%s, ", creators[i].scene_name);
            } else {
                printf("%s\n", creators[i].scene_name);
            }
        }
    }
    return scene;
}

static mat4_t get_light_view_matrix(vec3_t light_dir) {
    vec3_t light_pos = vec3_negate(light_dir);
    vec3_t light_target = vec3_new(0, 0, 0);
    vec3_t light_up = vec3_new(0, 1, 0);
    return mat4_lookat(light_pos, light_target, light_up);
}

static mat4_t get_light_proj_matrix(float half_w, float half_h,
                                    float z_near, float z_far) {
    return mat4_orthographic(half_w, half_h, z_near, z_far);
}

perframe_t test_build_perframe(scene_t *scene, context_t *context) {
    vec3_t light_dir = vec3_normalize(context->light_dir);
    camera_t *camera = context->camera;
    perframe_t perframe;

    perframe.frame_time = context->frame_time;
    perframe.delta_time = context->delta_time;
    perframe.light_dir = light_dir;
    perframe.camera_pos = camera_get_position(camera);
    perframe.light_view_matrix = get_light_view_matrix(light_dir);
    perframe.light_proj_matrix = get_light_proj_matrix(1, 1, 0, 2);
    perframe.camera_view_matrix = camera_get_view_matrix(camera);
    perframe.camera_proj_matrix = camera_get_proj_matrix(camera);
    perframe.ambient_intensity = scene->ambient_intensity;
    perframe.punctual_intensity = scene->punctual_intensity;
    perframe.shadow_map = scene->shadow_map;
    perframe.layer_view = -1;

    return perframe;
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

static void sort_models(model_t **models, mat4_t view_matrix) {
    int num_models = darray_size(models);
    int i;
    if (num_models > 1) {
        for (i = 0; i < num_models; i++) {
            model_t *model = models[i];
            vec3_t center = mesh_get_center(model->mesh);
            vec4_t local_pos = vec4_from_vec3(center, 1);
            vec4_t world_pos = mat4_mul_vec4(model->transform, local_pos);
            vec4_t view_pos = mat4_mul_vec4(view_matrix, world_pos);
            model->distance = -view_pos.z;
        }
        qsort(models, num_models, sizeof(model_t*), compare_models);
    }
}

void test_draw_scene(scene_t *scene, framebuffer_t *framebuffer,
                     perframe_t *perframe) {
    model_t *skybox = scene->skybox;
    model_t **models = scene->models;
    int num_models = darray_size(models);
    int i;

    for (i = 0; i < num_models; i++) {
        model_t *model = models[i];
        model->update(model, perframe);
    }
    if (skybox != NULL) {
        skybox->update(skybox, perframe);
    }

    if (scene->shadow_buffer && scene->shadow_map) {
        sort_models(models, perframe->light_view_matrix);
        framebuffer_clear_depth(scene->shadow_buffer, 1);
        for (i = 0; i < num_models; i++) {
            model_t *model = models[i];
            if (model->opaque) {
                model->draw(model, scene->shadow_buffer, 1);
            }
        }
        texture_from_depthbuffer(scene->shadow_map, scene->shadow_buffer);
    }

    sort_models(models, perframe->camera_view_matrix);
    framebuffer_clear_color(framebuffer, scene->background);
    framebuffer_clear_depth(framebuffer, 1);
    if (skybox == NULL || perframe->layer_view >= 0) {
        for (i = 0; i < num_models; i++) {
            model_t *model = models[i];
            model->draw(model, framebuffer, 0);
        }
    } else {
        int num_opaques = 0;
        for (i = 0; i < num_models; i++) {
            model_t *model = models[i];
            if (model->opaque) {
                num_opaques += 1;
            } else {
                break;
            }
        }

        for (i = 0; i < num_opaques; i++) {
            model_t *model = models[i];
            model->draw(model, framebuffer, 0);
        }
        skybox->draw(skybox, framebuffer, 0);
        for (i = num_opaques; i < num_models; i++) {
            model_t *model = models[i];
            model->draw(model, framebuffer, 0);
        }
    }
}
