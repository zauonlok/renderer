#ifndef SCENE_H
#define SCENE_H

#include "geometry.h"
#include "graphics.h"
#include "mesh.h"

typedef struct model model_t;
typedef struct scene scene_t;

struct model {
    mesh_t *mesh;
    mat4_t transform;
    program_t *program;
    /* generic functions */
    void (*draw)(model_t *model, framebuffer_t *framebuffer);
    void (*release)(model_t *model);
    /* for model sorting */
    int opaque;
    vec3_t center;
    float distance;
};

struct scene {
    vec4_t background;
    model_t *skybox;
    model_t **models;
    void *userdata;
};

#endif
