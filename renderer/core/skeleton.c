#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "geometry.h"
#include "skeleton.h"

/*
 * for skinned animation, see
 * https://people.rennes.inria.fr/Ludovic.Hoyet/teaching/IMO/05_IMO2016_Skinning.pdf
 */

typedef struct {
    int parent_index;
    mat4_t inverse_bind;
    /* translations */
    int num_translations;
    float *translation_times;
    vec3_t *translation_values;
    /* rotations */
    int num_rotations;
    float *rotation_times;
    quat_t *rotation_values;
    /* scales */
    int num_scales;
    float *scale_times;
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
        int time_size = (int)sizeof(float) * joint->num_translations;
        int value_size = (int)sizeof(vec3_t) * joint->num_translations;
        joint->translation_times = (float*)malloc(time_size);
        joint->translation_values = (vec3_t*)malloc(value_size);
        for (i = 0; i < joint->num_translations; i++) {
            read_line(file, line);
            items = sscanf(line, "        time: %f, value: [%f, %f, %f]",
                           &joint->translation_times[i],
                           &joint->translation_values[i].x,
                           &joint->translation_values[i].y,
                           &joint->translation_values[i].z);
            assert(items == 4);
        }
    } else {
        joint->translation_times = NULL;
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
        int time_size = (int)sizeof(float) * joint->num_rotations;
        int value_size = (int)sizeof(quat_t) * joint->num_rotations;
        joint->rotation_times = (float*)malloc(time_size);
        joint->rotation_values = (quat_t*)malloc(value_size);
        for (i = 0; i < joint->num_rotations; i++) {
            read_line(file, line);
            items = sscanf(line, "        time: %f, value: [%f, %f, %f, %f]",
                           &joint->rotation_times[i],
                           &joint->rotation_values[i].x,
                           &joint->rotation_values[i].y,
                           &joint->rotation_values[i].z,
                           &joint->rotation_values[i].w);
            assert(items == 5);
        }
    } else {
        joint->rotation_times = NULL;
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
        int time_size = (int)sizeof(float) * joint->num_scales;
        int value_size = (int)sizeof(vec3_t) * joint->num_scales;
        joint->scale_times = (float*)malloc(time_size);
        joint->scale_values = (vec3_t*)malloc(value_size);
        for (i = 0; i < joint->num_scales; i++) {
            read_line(file, line);
            items = sscanf(line, "        time: %f, value: [%f, %f, %f]",
                           &joint->scale_times[i],
                           &joint->scale_values[i].x,
                           &joint->scale_values[i].y,
                           &joint->scale_values[i].z);
            assert(items == 4);
        }
    } else {
        joint->scale_times = NULL;
        joint->scale_values = NULL;
    }
}

static joint_t load_joint(FILE *file) {
    char line[LINE_LENGTH];
    joint_t joint;
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
    char line[LINE_LENGTH];
    skeleton_t *skeleton;
    FILE *file;
    int items;
    int i;

    skeleton = (skeleton_t*)malloc(sizeof(skeleton_t));

    file = fopen(filename, "rb");
    assert(file != NULL);

    read_line(file, line);
    items = sscanf(line, "joint-size: %d", &skeleton->num_joints);
    assert(items == 1);
    assert(skeleton->num_joints > 0 && skeleton->num_joints <= MAX_JOINTS);

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
        free(joint->translation_times);
        free(joint->translation_values);
        free(joint->rotation_times);
        free(joint->rotation_values);
        free(joint->scale_times);
        free(joint->scale_values);
    }
    free(skeleton->joints);
}

/* joint updating/dumping */

static vec3_t get_translation(joint_t *joint, float input_time) {
    int num_translations = joint->num_translations;
    float *translation_times = joint->translation_times;
    vec3_t *translation_values = joint->translation_values;

    if (num_translations == 0) {
        return vec3_new(0, 0, 0);
    } else if (input_time <= translation_times[0]) {
        return translation_values[0];
    } else if (input_time >= translation_times[num_translations - 1]) {
        return translation_values[num_translations - 1];
    } else {
        int i;
        for (i = 0; i < num_translations - 1; i++) {
            float curr_time = translation_times[i];
            float next_time = translation_times[i + 1];
            if (input_time >= curr_time && input_time < next_time) {
                float t = (input_time - curr_time) / (next_time - curr_time);
                vec3_t curr_translation = translation_values[i];
                vec3_t next_translation = translation_values[i + 1];
                return vec3_lerp(curr_translation, next_translation, t);
            }
        }
        assert(0);
        return vec3_new(0, 0, 0);
    }
}

static quat_t get_rotationn(joint_t *joint, float input_time) {
    int num_rotations = joint->num_rotations;
    float *rotation_times = joint->rotation_times;
    quat_t *rotation_values = joint->rotation_values;

    if (num_rotations == 0) {
        return quat_new(0, 0, 0, 1);
    } else if (input_time <= rotation_times[0]) {
        return rotation_values[0];
    } else if (input_time >= rotation_times[num_rotations - 1]) {
        return rotation_values[num_rotations - 1];
    } else {
        int i;
        for (i = 0; i < num_rotations - 1; i++) {
            float curr_time = rotation_times[i];
            float next_time = rotation_times[i + 1];
            if (input_time >= curr_time && input_time < next_time) {
                float t = (input_time - curr_time) / (next_time - curr_time);
                quat_t curr_rotation = rotation_values[i];
                quat_t next_rotation = rotation_values[i + 1];
                return quat_slerp(curr_rotation, next_rotation, t);
            }
        }
        assert(0);
        return quat_new(0, 0, 0, 1);
    }
}

static vec3_t get_scale(joint_t *joint, float input_time) {
    int num_scales = joint->num_scales;
    float *scale_times = joint->scale_times;
    vec3_t *scale_values = joint->scale_values;

    if (num_scales == 0) {
        return vec3_new(1, 1, 1);
    } else if (input_time <= scale_times[0]) {
        return scale_values[0];
    } else if (input_time >= scale_times[num_scales - 1]) {
        return scale_values[num_scales - 1];
    } else {
        int i;
        for (i = 0; i < num_scales - 1; i++) {
            float curr_time = scale_times[i];
            float next_time = scale_times[i + 1];
            if (input_time >= curr_time && input_time < next_time) {
                float t = (input_time - curr_time) / (next_time - curr_time);
                vec3_t curr_scale = scale_values[i];
                vec3_t next_scale = scale_values[i + 1];
                return vec3_lerp(curr_scale, next_scale, t);
            }
        }
        assert(0);
        return vec3_new(1, 1, 1);
    }
}

void skeleton_update_joints(skeleton_t *skeleton, float input_time) {
    int i;
    input_time = (float)fmod(input_time, skeleton->max_time);
    for (i = 0; i < skeleton->num_joints; i++) {
        joint_t *joint = &skeleton->joints[i];
        vec3_t translation = get_translation(joint, input_time);
        quat_t rotation = get_rotationn(joint, input_time);
        vec3_t scale = get_scale(joint, input_time);
        joint->transform = mat4_from_trs(translation, rotation, scale);
        if (joint->parent_index >= 0) {
            joint_t *parent_joint = &skeleton->joints[joint->parent_index];
            joint->transform = mat4_mul_mat4(parent_joint->transform,
                                             joint->transform);
        }
    }
}

void skeleton_dump_matrices(skeleton_t *skeleton, mat4_t matrices[MAX_JOINTS]) {
    int i;
    assert(skeleton->num_joints <= MAX_JOINTS);
    for (i = 0; i < skeleton->num_joints; i++) {
        joint_t *joint = &skeleton->joints[i];
        matrices[i] = mat4_mul_mat4(joint->transform, joint->inverse_bind);
    }
}
