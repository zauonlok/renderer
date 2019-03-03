#ifndef MESH_H
#define MESH_H

#include "geometry.h"

typedef struct mesh mesh_t;

typedef struct {
    vec3_t position;
    vec2_t texcoord;
    vec3_t normal;
    vec4_t tangent;
    vec4_t joint;
    vec4_t weight;
} vertex_t;

/* mesh loading/releasing */
mesh_t *mesh_load(const char *filename);
void mesh_release(mesh_t *mesh);

/* vertex retrieving */
int mesh_get_num_faces(mesh_t *mesh);
vertex_t mesh_get_vertex(mesh_t *mesh, int nth_face, int nth_vertex);

#endif
