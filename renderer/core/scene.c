#include <stdlib.h>
#include "darray.h"
#include "graphics.h"
#include "maths.h"
#include "mesh.h"
#include "scene.h"
#include "skeleton.h"
#include "texture.h"

scene_t *scene_create(vec3_t background, model_t *skybox, model_t **models,
                      float ambient_intensity, float punctual_intensity,
                      int shadow_width, int shadow_height) {
    scene_t *scene = (scene_t*)malloc(sizeof(scene_t));
    scene->background = vec4_from_vec3(background, 1);
    scene->skybox = skybox;
    scene->models = models;
    scene->ambient_intensity = ambient_intensity;
    scene->punctual_intensity = punctual_intensity;
    if (shadow_width > 0 && shadow_height > 0) {
        scene->shadow_buffer = framebuffer_create(shadow_width, shadow_height);
        scene->shadow_map = texture_create(shadow_width, shadow_height);
    } else {
        scene->shadow_buffer = NULL;
        scene->shadow_map = NULL;
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
    if (scene->shadow_buffer) {
        framebuffer_release(scene->shadow_buffer);
    }
    if (scene->shadow_map) {
        texture_release(scene->shadow_map);
    }
    free(scene);
}
