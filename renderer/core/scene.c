#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "darray.h"
#include "geometry.h"
#include "graphics.h"
#include "mesh.h"
#include "scene.h"

/* scene creating/releasing */

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
            vec3_t local_pos = mesh_get_position(mesh, i, j);
            vec3_t world_pos = transform_position(local_pos, transform);
            *bbmin = vec3_min(*bbmin, world_pos);
            *bbmax = vec3_max(*bbmax, world_pos);
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

scene_t *scene_create(creator_t creators[], const char *scene_name) {
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
    for (i = 0; i < num_models; i++) {
        model_t *model = scene->models[i];
        model->release(model);
    }
    darray_free(scene->models);
    free(scene);
}

/* model sorting */

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
    for (i = 0; i < num_models; i++) {
        model_t *model = scene->models[i];
        vec4_t local_pos = vec4_from_vec3(model->center, 1);
        vec4_t world_pos = mat4_mul_vec4(model->transform, local_pos);
        vec4_t view_pos = mat4_mul_vec4(view_matrix, world_pos);
        model->distance = -view_pos.z;
    }
    qsort(scene->models, num_models, sizeof(model_t*), compare_models);
}
