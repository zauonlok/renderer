#ifndef SKELETON_H
#define SKELETON_H

#include "maths.h"

typedef struct skeleton skeleton_t;

/* skeleton loading/releasing */
skeleton_t *skeleton_load(const char *filename);
void skeleton_release(skeleton_t *skeleton);

/* joint updating/retrieving */
void skeleton_update_joints(skeleton_t *skeleton, float frame_time);
mat4_t *skeleton_get_joint_matrices(skeleton_t *skeleton);
mat3_t *skeleton_get_normal_matrices(skeleton_t *skeleton);

#endif
