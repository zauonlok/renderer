#include <stdlib.h>
#include "darray.h"
#include "geometry.h"
#include "graphics.h"
#include "mesh.h"
#include "scene.h"
#include "skeleton.h"
#include "texture.h"

static const int SHADOWMAP_WIDTH = 512;
static const int SHADOWMAP_HEIGHT = 512;

scene_t *scene_create(vec3_t background, model_t *skybox, model_t **models,
                      float ambient_intensity, float punctual_intensity,
                      int with_shadow) {
    scene_t *scene = (scene_t*)malloc(sizeof(scene_t));
    scene->background = vec4_from_vec3(background, 1);
    scene->skybox = skybox;
    scene->models = models;
    scene->ambient_intensity = ambient_intensity;
    scene->punctual_intensity = punctual_intensity;
    if (with_shadow) {
        scene->shadow_buffer = framebuffer_create(SHADOWMAP_WIDTH,
                                                  SHADOWMAP_HEIGHT);
        scene->shadow_map = texture_create(SHADOWMAP_WIDTH,
                                           SHADOWMAP_HEIGHT);
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

void scene_draw(scene_t *scene, framebuffer_t *framebuffer,
                framedata_t *framedata) {
    model_t *skybox = scene->skybox;
    model_t **models = scene->models;
    int num_models = darray_size(models);
    int i;

    for (i = 0; i < num_models; i++) {
        model_t *model = models[i];
        model->update(model, framedata);
    }
    if (skybox != NULL) {
        skybox->update(skybox, framedata);
    }

    if (scene->shadow_buffer && scene->shadow_map) {
        sort_models(models, framedata->light_view_matrix);
        framebuffer_clear_depth(scene->shadow_buffer, 1);
        for (i = 0; i < num_models; i++) {
            model_t *model = models[i];
            if (model->opaque) {
                model->draw(model, scene->shadow_buffer, 1);
            }
        }
        texture_from_depth(scene->shadow_map, scene->shadow_buffer);
    }

    sort_models(models, framedata->camera_view_matrix);
    framebuffer_clear_color(framebuffer, scene->background);
    framebuffer_clear_depth(framebuffer, 1);
    if (skybox == NULL) {
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
