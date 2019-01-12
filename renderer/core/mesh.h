#ifndef MESH_H
#define MESH_H

#include "geometry.h"

typedef struct mesh mesh_t;

/* mesh loading/releasing */
mesh_t *mesh_load(const char *filename);
void mesh_release(mesh_t *mesh);

/* vertex retrieving */
int mesh_get_num_faces(mesh_t *mesh);
vec3_t mesh_get_position(mesh_t *mesh, int nth_face, int nth_position);
vec2_t mesh_get_texcoord(mesh_t *mesh, int nth_face, int nth_texcoord);
vec3_t mesh_get_normal(mesh_t *mesh, int nth_face, int nth_normal);
vec4_t mesh_get_tangent(mesh_t *mesh, int nth_face, int nth_tangent);

#endif
