#ifndef TEST_BASE_H
#define TEST_BASE_H

#include "../core/api.h"

/* test delegate functions */

typedef struct {
    framebuffer_t *framebuffer;
    camera_t *camera;
    vec3_t light_dir;
    float delta_time;
} context_t;

typedef void tickfunc_t(context_t *context, void *userdata);

void test_base(tickfunc_t *tickfunc, void *userdata);

/* scene helper functions */

typedef struct {
    const char *scene_name;
    scene_t *(*scene_ctor)(void);
} scene_entry_t;

scene_t *scene_create(scene_entry_t scene_entries[], int num_entries,
                      const char *scene_name);
void scene_release(scene_t *scene, void (*model_dtor)(model_t *model));

#endif
