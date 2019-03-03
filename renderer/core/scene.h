#ifndef SCENE_H
#define SCENE_H

#include "geometry.h"
#include "graphics.h"
#include "mesh.h"

typedef struct model model_t;
typedef struct scene scene_t;

typedef void model_release_t(model_t *model);
typedef scene_t *scene_create_t(void);

typedef struct {
    const char *scene_name;
    scene_create_t *create;
} creator_t;

struct model {
    mesh_t *mesh;
    mat4_t transform;
    program_t *program;
    model_release_t *release;
    /* for model sorting */
    int opaque;
    vec3_t center;
    float distance;
};

struct scene {
    vec4_t background;
    model_t **models;
    void *userdata;
};

/* scene creating/releasing */
scene_t *scene_create(creator_t creators[], const char *scene_name);
void scene_release(scene_t *scene);

/* model sorting */
void scene_sort_models(scene_t *scene, mat4_t view_matrix);

#endif
