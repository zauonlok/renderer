#ifndef MODEL_H
#define MODEL_H

#include "geometry.h"

typedef struct model model_t;

/* model loading */
model_t *model_load(const char *filename);
void model_free(model_t *model);

/* vertex retrieving */
int model_get_num_faces(model_t *model);
vec3_t model_get_position(model_t *model, int nth_face, int nth_position);
vec2_t model_get_texcoord(model_t *model, int nth_face, int nth_texcoord);
vec3_t model_get_normal(model_t *model, int nth_face, int nth_normal);

#endif
