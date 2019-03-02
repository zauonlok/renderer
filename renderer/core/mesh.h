#ifndef MESH_H
#define MESH_H

#include "geometry.h"

typedef struct mesh mesh_t;
typedef struct skin skin_t;

/* mesh loading/releasing */
mesh_t *mesh_load(const char *filename);
void mesh_release(mesh_t *mesh);

/* vertex retrieving */
int mesh_get_num_faces(mesh_t *mesh);
vec3_t mesh_get_position(mesh_t *mesh, int nth_face, int nth_position);
vec2_t mesh_get_texcoord(mesh_t *mesh, int nth_face, int nth_texcoord);
vec3_t mesh_get_normal(mesh_t *mesh, int nth_face, int nth_normal);
vec4_t mesh_get_tangent(mesh_t *mesh, int nth_face, int nth_tangent);
vec4_t mesh_get_joint(mesh_t *mesh, int nth_face, int nth_joint);
vec4_t mesh_get_weight(mesh_t *mesh, int nth_face, int nth_weight);

/* skin loading/releasing */
skin_t *skin_load(const char *filename);
void skin_release(skin_t *skin);

/* joint updating/dumping */
void skin_update_joints(skin_t *skin, float timer);
void skin_dump_matrices(skin_t *skin, mat4_t matrices[256]);

#endif
