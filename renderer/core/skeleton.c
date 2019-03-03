#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "geometry.h"
#include "skeleton.h"

typedef struct {
    int parent_index;
    mat4_t inverse_bind;
    /* translations */
    int num_translations;
    float *translation_timers;
    vec3_t *translation_values;
    /* rotations */
    int num_rotations;
    float *rotation_timers;
    quat_t *rotation_values;
    /* scales */
    int num_scales;
    float *scale_timers;
    vec3_t *scale_values;
    /* interpolated */
    mat4_t transform;
} joint_t;

struct skeleton {
    float min_time;
    float max_time;
    int num_joints;
    joint_t *joints;
};

/* skeleton loading/releasing */

#define LINE_LENGTH 1024

static void read_line(FILE *file, char line[LINE_LENGTH]) {
    if (fgets(line, LINE_LENGTH, file) == NULL) {
        assert(0);
    }
}

static void read_parent_index(FILE *file, joint_t *joint) {
    char line[LINE_LENGTH];
    int items;
    read_line(file, line);
    items = sscanf(line, "    parent-index: %d", &joint->parent_index);
    assert(items == 1);
}

static void read_inverse_bind(FILE *file, joint_t *joint) {
    char line[LINE_LENGTH];
    int items;
    int i;
    read_line(file, line);
    assert(strstr(line, "    inverse-bind:") != NULL);
    for (i = 0; i < 4; i++) {
        read_line(file, line);
        items = sscanf(line, "        %f %f %f %f",
                       &joint->inverse_bind.m[i][0],
                       &joint->inverse_bind.m[i][1],
                       &joint->inverse_bind.m[i][2],
                       &joint->inverse_bind.m[i][3]);
        assert(items == 4);
    }
}

static void read_translations(FILE *file, joint_t *joint) {
    char line[LINE_LENGTH];
    int items;
    int i;
    read_line(file, line);
    items = sscanf(line, "    translations %d:", &joint->num_translations);
    assert(items == 1 && joint->num_translations >= 0);
    if (joint->num_translations > 0) {
        int timer_size = (int)sizeof(float) * joint->num_translations;
        int value_size = (int)sizeof(vec3_t) * joint->num_translations;
        joint->translation_timers = (float*)malloc(timer_size);
        joint->translation_values = (vec3_t*)malloc(value_size);
        for (i = 0; i < joint->num_translations; i++) {
            read_line(file, line);
            items = sscanf(line, "        time: %f, value: [%f, %f, %f]",
                           &joint->translation_timers[i],
                           &joint->translation_values[i].x,
                           &joint->translation_values[i].y,
                           &joint->translation_values[i].z);
            assert(items == 4);
        }
    } else {
        joint->translation_timers = NULL;
        joint->translation_values = NULL;
    }
}

static void read_rotations(FILE *file, joint_t *joint) {
    char line[LINE_LENGTH];
    int items;
    int i;
    read_line(file, line);
    items = sscanf(line, "    rotations %d:", &joint->num_rotations);
    assert(items == 1 && joint->num_rotations >= 0);
    if (joint->num_rotations > 0) {
        int timer_size = (int)sizeof(float) * joint->num_rotations;
        int value_size = (int)sizeof(quat_t) * joint->num_rotations;
        joint->rotation_timers = (float*)malloc(timer_size);
        joint->rotation_values = (quat_t*)malloc(value_size);
        for (i = 0; i < joint->num_rotations; i++) {
            read_line(file, line);
            items = sscanf(line, "        time: %f, value: [%f, %f, %f, %f]",
                           &joint->rotation_timers[i],
                           &joint->rotation_values[i].x,
                           &joint->rotation_values[i].y,
                           &joint->rotation_values[i].z,
                           &joint->rotation_values[i].w);
            assert(items == 5);
        }
    } else {
        joint->rotation_timers = NULL;
        joint->rotation_values = NULL;
    }
}

static void read_scales(FILE *file, joint_t *joint) {
    char line[LINE_LENGTH];
    int items;
    int i;
    read_line(file, line);
    items = sscanf(line, "    scales %d:", &joint->num_scales);
    assert(items == 1 && joint->num_scales >= 0);
    if (joint->num_scales > 0) {
        int timer_size = (int)sizeof(float) * joint->num_scales;
        int value_size = (int)sizeof(vec3_t) * joint->num_scales;
        joint->scale_timers = (float*)malloc(timer_size);
        joint->scale_values = (vec3_t*)malloc(value_size);
        for (i = 0; i < joint->num_scales; i++) {
            read_line(file, line);
            items = sscanf(line, "        time: %f, value: [%f, %f, %f]",
                           &joint->scale_timers[i],
                           &joint->scale_values[i].x,
                           &joint->scale_values[i].y,
                           &joint->scale_values[i].z);
            assert(items == 4);
        }
    } else {
        joint->scale_timers = NULL;
        joint->scale_values = NULL;
    }
}

static joint_t load_joint(FILE *file) {
    joint_t joint;
    char line[LINE_LENGTH];
    int items;
    int dummy;

    while (1) {
        read_line(file, line);
        if (line[0] != '\0' && line[0] != '\r' && line[0] != '\n') {
            break;
        }
    }
    items = sscanf(line, "joint %d:", &dummy);
    assert(items == 1);

    read_parent_index(file, &joint);
    read_inverse_bind(file, &joint);
    read_translations(file, &joint);
    read_rotations(file, &joint);
    read_scales(file, &joint);

    return joint;
}

static skeleton_t *load_ani(const char *filename) {
    skeleton_t *skeleton;
    FILE *file;
    char line[LINE_LENGTH];
    int items;
    int i;

    skeleton = (skeleton_t*)malloc(sizeof(skeleton_t));

    file = fopen(filename, "rb");
    assert(file != NULL);

    read_line(file, line);
    items = sscanf(line, "joint-size: %d", &skeleton->num_joints);
    assert(items == 1 && skeleton->num_joints > 0);

    read_line(file, line);
    items = sscanf(line, "time-range: [%f, %f]",
                   &skeleton->min_time, &skeleton->max_time);
    assert(items == 2 && skeleton->min_time < skeleton->max_time);

    skeleton->joints = (joint_t*)malloc(sizeof(joint_t) * skeleton->num_joints);
    for (i = 0; i < skeleton->num_joints; i++) {
        skeleton->joints[i] = load_joint(file);
    }

    fclose(file);

    return skeleton;
}

static const char *extract_extension(const char *filename) {
    const char *dot_pos = strrchr(filename, '.');
    return dot_pos == NULL ? "" : dot_pos + 1;
}

skeleton_t *skeleton_load(const char *filename) {
    const char *extension = extract_extension(filename);
    if (strcmp(extension, "ani") == 0) {
        return load_ani(filename);
    } else {
        assert(0);
        return NULL;
    }
}

void skeleton_release(skeleton_t *skeleton) {
    int i;
    for (i = 0; i < skeleton->num_joints; i++) {
        joint_t *joint = &skeleton->joints[i];
        free(joint->translation_timers);
        free(joint->translation_values);
        free(joint->rotation_timers);
        free(joint->rotation_values);
        free(joint->scale_timers);
        free(joint->scale_values);
    }
    free(skeleton->joints);
}

/* joint updating/dumping */

void skeleton_update_joints(skeleton_t *skeleton, float timer) {
    int i, j;
    timer = (float)fmod(timer, skeleton->max_time);
    for (i = 0; i < skeleton->num_joints; i++) {
        joint_t *joint = &skeleton->joints[i];
        vec3_t translation = vec3_new(0, 0, 0);
        quat_t rotation = quat_new(0, 0, 0, 1);
        vec3_t scale = vec3_new(1, 1, 1);
        for (j = 0; j < joint->num_translations - 1; j++) {
            float curr_timer = joint->translation_timers[j];
            float next_timer = joint->translation_timers[j + 1];
            if (timer >= curr_timer && timer <= next_timer) {
                float t = (timer - curr_timer) / (next_timer - curr_timer);
                vec3_t curr_translation = joint->translation_values[j];
                vec3_t next_translation = joint->translation_values[j + 1];
                translation = vec3_lerp(curr_translation, next_translation, t);
                break;
            }
        }
        for (j = 0; j < joint->num_rotations - 1; j++) {
            float curr_timer = joint->rotation_timers[j];
            float next_timer = joint->rotation_timers[j + 1];
            if (timer >= curr_timer && timer <= next_timer) {
                float t = (timer - curr_timer) / (next_timer - curr_timer);
                quat_t curr_rotation = joint->rotation_values[j];
                quat_t next_rotation = joint->rotation_values[j + 1];
                rotation = quat_slerp(curr_rotation, next_rotation, t);
                break;
            }
        }
        for (j = 0; j < joint->num_scales - 1; j++) {
            float curr_timer = joint->scale_timers[j];
            float next_timer = joint->scale_timers[j + 1];
            if (timer >= curr_timer && timer <= next_timer) {
                float t = (timer - curr_timer) / (next_timer - curr_timer);
                vec3_t curr_scale = joint->scale_values[j];
                vec3_t next_scale = joint->scale_values[j + 1];
                scale = vec3_lerp(curr_scale, next_scale, t);
                break;
            }
        }
        joint->transform = mat4_from_trs(translation, rotation, scale);
        if (joint->parent_index >= 0) {
            joint_t *parent_joint = &skeleton->joints[joint->parent_index];
            joint->transform = mat4_mul_mat4(parent_joint->transform,
                                             joint->transform);
        }
    }
}

void skeleton_dump_matrices(skeleton_t *skeleton, mat4_t matrices[256]) {
    int i;
    assert(skeleton->num_joints <= 256);
    for (i = 0; i < skeleton->num_joints; i++) {
        joint_t *joint = &skeleton->joints[i];
        matrices[i] = mat4_mul_mat4(joint->transform, joint->inverse_bind);
    }
}
