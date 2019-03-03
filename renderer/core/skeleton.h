#ifndef SKELETON_H
#define SKELETON_H

#include "geometry.h"

typedef struct skeleton skeleton_t;

/* skeleton loading/releasing */
skeleton_t *skeleton_load(const char *filename);
void skeleton_release(skeleton_t *skeleton);

/* joint updating/dumping */
void skeleton_update_joints(skeleton_t *skeleton, float timer);
void skeleton_dump_matrices(skeleton_t *skeleton, mat4_t matrices[256]);

#endif
