#include <stdlib.h>
#include "darray.h"
#include "geometry.h"
#include "graphics.h"
#include "mesh.h"
#include "scene.h"
#include "skeleton.h"
#include "texture.h"

scene_t *scene_create(vec4_t background, model_t *skybox, model_t **models,
                      float ambient_light, float punctual_light,
                      int with_shadow) {
    scene_t *scene = (scene_t*)malloc(sizeof(scene_t));
    scene->background          = background;
    scene->skybox              = skybox;
    scene->models              = models;
    scene->light_info.ambient  = ambient_light;
    scene->light_info.punctual = punctual_light;
    scene->with_shadow         = with_shadow;
    scene->shadow_fb           = NULL;
    scene->shadow_map          = NULL;
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
    if (scene->shadow_fb) {
        framebuffer_release(scene->shadow_fb);
    }
    if (scene->shadow_map) {
        texture_release(scene->shadow_map);
    }
    free(scene);
}
