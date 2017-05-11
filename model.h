#ifndef MODEL_H
#define MODEL_H

#include "geometry.h"

typedef struct model model_t;

model_t *model_load(const char *filename);
void model_free(model_t *model);

int model_get_num_faces(model_t *model);
vec3f_t model_get_vertex(model_t *model, int face, int nth_vertex);
vec2f_t model_get_uv(model_t *model, int face, int nth_uv);
vec3f_t model_get_normal(model_t *model, int face, int nth_normal);

#endif
