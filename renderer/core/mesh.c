#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "darray.h"
#include "geometry.h"
#include "mesh.h"

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

struct mesh {
    int num_faces;
    vec3_t *positions;
    vec2_t *texcoords;
    vec3_t *normals;
    vec4_t *tangents;
    vec4_t *joints;
    vec4_t *weights;
};

struct skin {
    float min_time;
    float max_time;
    int num_joints;
    joint_t *joints;
};

/* mesh loading/releasing */

static char *read_line(FILE *file) {
    char *line = NULL;

    while (1) {
        int c = fgetc(file);
        if (c == EOF) {
            break;
        }
        darray_push(line, (char)c);
        if (c == '\n') {
            break;
        }
    }

    if (line == NULL) {
        return NULL;
    } else {
        darray_push(line, '\0');
        return line;
    }
}

static mesh_t *build_mesh(
        vec3_t *positions, vec2_t *texcoords, vec3_t *normals,
        vec4_t *tangents, vec4_t *joints, vec4_t *weights,
        int *position_indices, int *texcoord_indices, int *normal_indices) {
    int num_indices = darray_size(position_indices);
    int num_faces = num_indices / 3;
    mesh_t *mesh;
    int i;

    assert(num_faces > 0 && num_faces * 3 == num_indices);
    assert(darray_size(position_indices) == num_indices);
    assert(darray_size(texcoord_indices) == num_indices);
    assert(darray_size(normal_indices) == num_indices);

    mesh = (mesh_t*)malloc(sizeof(mesh_t));
    mesh->num_faces = num_faces;
    mesh->positions = (vec3_t*)malloc(sizeof(vec3_t) * num_indices);
    mesh->texcoords = (vec2_t*)malloc(sizeof(vec2_t) * num_indices);
    mesh->normals   = (vec3_t*)malloc(sizeof(vec3_t) * num_indices);
    mesh->tangents  = NULL;
    mesh->joints    = NULL;
    mesh->weights   = NULL;

    for (i = 0; i < num_indices; i++) {
        int position_index = position_indices[i];
        int texcoord_index = texcoord_indices[i];
        int normal_index = normal_indices[i];
        assert(position_index >= 0 && position_index < darray_size(positions));
        assert(texcoord_index >= 0 && texcoord_index < darray_size(texcoords));
        assert(normal_index >= 0 && normal_index < darray_size(normals));
        mesh->positions[i] = positions[position_index];
        mesh->texcoords[i] = texcoords[texcoord_index];
        mesh->normals[i] = normals[normal_index];
    }

    if (tangents) {
        mesh->tangents = (vec4_t*)malloc(sizeof(vec4_t) * num_indices);
        for (i = 0; i < num_indices; i++) {
            int tangent_index = position_indices[i];  /* use position indices */
            assert(tangent_index >= 0 && tangent_index < darray_size(tangents));
            mesh->tangents[i] = tangents[tangent_index];
        }
    }

    if (joints) {
        mesh->joints = (vec4_t*)malloc(sizeof(vec4_t) * num_indices);
        for (i = 0; i < num_indices; i++) {
            int joint_index = position_indices[i];    /* use position indices */
            assert(joint_index >= 0 && joint_index < darray_size(joints));
            mesh->joints[i] = joints[joint_index];
        }
    }

    if (weights) {
        mesh->weights = (vec4_t*)malloc(sizeof(vec4_t) * num_indices);
        for (i = 0; i < num_indices; i++) {
            int weight_index = position_indices[i];   /* use position indices */
            assert(weight_index >= 0 && weight_index < darray_size(weights));
            mesh->weights[i] = weights[weight_index];
        }
    }

    return mesh;
}

static mesh_t *load_obj(const char *filename) {
    vec3_t *positions = NULL;
    vec2_t *texcoords = NULL;
    vec3_t *normals = NULL;
    vec4_t *tangents = NULL;
    vec4_t *joints = NULL;
    vec4_t *weights = NULL;
    int *position_indices = NULL;
    int *texcoord_indices = NULL;
    int *normal_indices = NULL;
    mesh_t *mesh;
    FILE *file;

    file = fopen(filename, "rb");
    assert(file != NULL);
    while (1) {
        int items;
        char *line = read_line(file);
        if (line == NULL) {
            break;
        }
        if (strncmp(line, "v ", 2) == 0) {                      /* position */
            vec3_t position;
            items = sscanf(line + 2, "%f %f %f",
                           &position.x, &position.y, &position.z);
            assert(items == 3);
            darray_push(positions, position);
        } else if (strncmp(line, "vt ", 3) == 0) {              /* texcoord */
            vec2_t texcoord;
            items = sscanf(line + 3, "%f %f", &texcoord.x, &texcoord.y);
            assert(items == 2);
            darray_push(texcoords, texcoord);
        } else if (strncmp(line, "vn ", 3) == 0) {              /* normal */
            vec3_t normal;
            items = sscanf(line + 3, "%f %f %f",
                           &normal.x, &normal.y, &normal.z);
            assert(items == 3);
            darray_push(normals, normal);
        } else if (strncmp(line, "f ", 2) == 0) {               /* face */
            int i;
            int pos_indices[3], uv_indices[3], nor_indices[3];
            items = sscanf(line + 2, "%d/%d/%d %d/%d/%d %d/%d/%d",
                           &pos_indices[0], &uv_indices[0], &nor_indices[0],
                           &pos_indices[1], &uv_indices[1], &nor_indices[1],
                           &pos_indices[2], &uv_indices[2], &nor_indices[2]);
            assert(items == 9);
            for (i = 0; i < 3; i++) {
                darray_push(position_indices, pos_indices[i] - 1);
                darray_push(texcoord_indices, uv_indices[i] - 1);
                darray_push(normal_indices, nor_indices[i] - 1);
            }
        } else if (strncmp(line, "# ext.tangent ", 14) == 0) {  /* tangent */
            vec4_t tangent;
            items = sscanf(line + 14, "%f %f %f %f",
                           &tangent.x, &tangent.y, &tangent.z, &tangent.w);
            assert(items == 4);
            darray_push(tangents, tangent);
        } else if (strncmp(line, "# ext.joint ", 12) == 0) {    /* joint */
            vec4_t joint;
            items = sscanf(line + 12, "%f %f %f %f",
                           &joint.x, &joint.y, &joint.z, &joint.w);
            assert(items == 4);
            darray_push(joints, joint);
        } else if (strncmp(line, "# ext.weight ", 13) == 0) {   /* weight */
            vec4_t weight;
            items = sscanf(line + 13, "%f %f %f %f",
                           &weight.x, &weight.y, &weight.z, &weight.w);
            assert(items == 4);
            darray_push(weights, weight);
        }
        darray_free(line);
    }
    fclose(file);

    mesh = build_mesh(positions, texcoords, normals, tangents, joints, weights,
                      position_indices, texcoord_indices, normal_indices);
    darray_free(positions);
    darray_free(texcoords);
    darray_free(normals);
    darray_free(tangents);
    darray_free(joints);
    darray_free(weights);
    darray_free(position_indices);
    darray_free(texcoord_indices);
    darray_free(normal_indices);

    return mesh;
}

static const char *extract_extension(const char *filename) {
    const char *dot_pos = strrchr(filename, '.');
    return dot_pos == NULL ? "" : dot_pos + 1;
}

mesh_t *mesh_load(const char *filename) {
    const char *extension = extract_extension(filename);
    if (strcmp(extension, "obj") == 0) {
        return load_obj(filename);
    } else {
        assert(0);
        return NULL;
    }
}

void mesh_release(mesh_t *mesh) {
    free(mesh->positions);
    free(mesh->texcoords);
    free(mesh->normals);
    free(mesh->tangents);
    free(mesh->joints);
    free(mesh->weights);
    free(mesh);
}

/* vertex retrieving */

int mesh_get_num_faces(mesh_t *mesh) {
    return mesh->num_faces;
}

#define validate_vertex(mesh, nth_face, nth_vertex)                         \
    do {                                                                    \
        assert((nth_face) >= 0 && (nth_face) < (mesh)->num_faces);          \
        assert((nth_vertex) >= 0 && (nth_vertex) < 3);                      \
    } while (0)

vec3_t mesh_get_position(mesh_t *mesh, int nth_face, int nth_position) {
    int index = nth_face * 3 + nth_position;
    validate_vertex(mesh, nth_face, nth_position);
    return mesh->positions[index];
}

vec2_t mesh_get_texcoord(mesh_t *mesh, int nth_face, int nth_texcoord) {
    int index = nth_face * 3 + nth_texcoord;
    validate_vertex(mesh, nth_face, nth_texcoord);
    return mesh->texcoords[index];
}

vec3_t mesh_get_normal(mesh_t *mesh, int nth_face, int nth_normal) {
    int index = nth_face * 3 + nth_normal;
    validate_vertex(mesh, nth_face, nth_normal);
    return mesh->normals[index];
}

vec4_t mesh_get_tangent(mesh_t *mesh, int nth_face, int nth_tangent) {
    if (mesh->tangents != NULL) {
        int index = nth_face * 3 + nth_tangent;
        validate_vertex(mesh, nth_face, nth_tangent);
        return mesh->tangents[index];
    } else {
        return vec4_new(0, 0, 0, 0);
    }
}

vec4_t mesh_get_joint(mesh_t *mesh, int nth_face, int nth_joint) {
    if (mesh->joints != NULL) {
        int index = nth_face * 3 + nth_joint;
        validate_vertex(mesh, nth_face, nth_joint);
        return mesh->joints[index];
    } else {
        return vec4_new(0, 0, 0, 0);
    }
}

vec4_t mesh_get_weight(mesh_t *mesh, int nth_face, int nth_weight) {
    if (mesh->weights != NULL) {
        int index = nth_face * 3 + nth_weight;
        validate_vertex(mesh, nth_face, nth_weight);
        return mesh->weights[index];
    } else {
        return vec4_new(0, 0, 0, 0);
    }
}

/* skin loading/releasing */

static char *expect_line(FILE *file) {
    char *line = read_line(file);
    assert(line != NULL);
    return line;
}

static joint_t load_joint(FILE *file) {
    joint_t joint;
    char *line;
    int items;
    int dummy;
    int i;

    while (1) {
        line = expect_line(file);
        if (strlen(line) == 0 || line[0] == '\r' || line[0] == '\n') {
            darray_free(line);
        } else {
            break;
        }
    }
    items = sscanf(line, "joint %d:", &dummy);
    assert(items == 1);
    darray_free(line);

    line = expect_line(file);
    items = sscanf(line, "    parent-index: %d", &joint.parent_index);
    assert(items == 1);
    darray_free(line);

    line = expect_line(file);
    assert(strstr(line, "    inverse-bind:") != NULL);
    darray_free(line);
    for (i = 0; i < 4; i++) {
        line = expect_line(file);
        items = sscanf(
            line, "        %f %f %f %f",
            &joint.inverse_bind.m[i][0], &joint.inverse_bind.m[i][1],
            &joint.inverse_bind.m[i][2], &joint.inverse_bind.m[i][3]);
        assert(items == 4);
        darray_free(line);
    }

    line = expect_line(file);
    items = sscanf(line, "    translations %d:", &joint.num_translations);
    assert(items == 1 && joint.num_translations >= 0);
    darray_free(line);
    if (joint.num_translations > 0) {
        int timer_size = (int)sizeof(float) * joint.num_translations;
        int value_size = (int)sizeof(vec3_t) * joint.num_translations;
        joint.translation_timers = (float*)malloc(timer_size);
        joint.translation_values = (vec3_t*)malloc(value_size);
        for (i = 0; i < joint.num_translations; i++) {
            line = expect_line(file);
            items = sscanf(line, "        time: %f, value: [%f, %f, %f]",
                           &joint.translation_timers[i],
                           &joint.translation_values[i].x,
                           &joint.translation_values[i].y,
                           &joint.translation_values[i].z);
            assert(items == 4);
            darray_free(line);
        }
    } else {
        joint.translation_timers = NULL;
        joint.translation_values = NULL;
    }

    line = expect_line(file);
    items = sscanf(line, "    rotations %d:", &joint.num_rotations);
    assert(items == 1 && joint.num_rotations >= 0);
    darray_free(line);
    if (joint.num_rotations > 0) {
        int timer_size = (int)sizeof(float) * joint.num_rotations;
        int value_size = (int)sizeof(quat_t) * joint.num_rotations;
        joint.rotation_timers = (float*)malloc(timer_size);
        joint.rotation_values = (quat_t*)malloc(value_size);
        for (i = 0; i < joint.num_rotations; i++) {
            line = expect_line(file);
            items = sscanf(line, "        time: %f, value: [%f, %f, %f, %f]",
                           &joint.rotation_timers[i],
                           &joint.rotation_values[i].x,
                           &joint.rotation_values[i].y,
                           &joint.rotation_values[i].z,
                           &joint.rotation_values[i].w);
            assert(items == 5);
            darray_free(line);
        }
    } else {
        joint.rotation_timers = NULL;
        joint.rotation_values = NULL;
    }

    line = expect_line(file);
    items = sscanf(line, "    scales %d:", &joint.num_scales);
    assert(items == 1 && joint.num_scales >= 0);
    darray_free(line);
    if (joint.num_scales > 0) {
        int timer_size = (int)sizeof(float) * joint.num_scales;
        int value_size = (int)sizeof(vec3_t) * joint.num_scales;
        joint.scale_timers = (float*)malloc(timer_size);
        joint.scale_values = (vec3_t*)malloc(value_size);
        for (i = 0; i < joint.num_scales; i++) {
            line = expect_line(file);
            items = sscanf(line, "        time: %f, value: [%f, %f, %f]",
                           &joint.scale_timers[i],
                           &joint.scale_values[i].x,
                           &joint.scale_values[i].y,
                           &joint.scale_values[i].z);
            assert(items == 4);
            darray_free(line);
        }
    } else {
        joint.scale_timers = NULL;
        joint.scale_values = NULL;
    }

    return joint;
}

static skin_t *load_ani(const char *filename) {
    skin_t *skin;
    FILE *file;
    char *line;
    int items;
    int i;

    skin = (skin_t*)malloc(sizeof(skin_t));

    file = fopen(filename, "rb");
    assert(file != NULL);

    line = expect_line(file);
    items = sscanf(line, "joint-size: %d", &skin->num_joints);
    assert(items == 1 && skin->num_joints > 0);
    darray_free(line);

    line = expect_line(file);
    items = sscanf(line, "time-range: [%f, %f]",
                   &skin->min_time, &skin->max_time);
    assert(items == 2 && skin->min_time < skin->max_time);
    darray_free(line);

    skin->joints = (joint_t*)malloc(sizeof(joint_t) * skin->num_joints);
    for (i = 0; i < skin->num_joints; i++) {
        skin->joints[i] = load_joint(file);
    }

    fclose(file);

    return skin;
}

skin_t *skin_load(const char *filename) {
    const char *extension = extract_extension(filename);
    if (strcmp(extension, "ani") == 0) {
        return load_ani(filename);
    } else {
        assert(0);
        return NULL;
    }
}

void skin_release(skin_t *skin) {
    int i;
    for (i = 0; i < skin->num_joints; i++) {
        joint_t joint = skin->joints[i];
        free(joint.translation_timers);
        free(joint.translation_values);
        free(joint.rotation_timers);
        free(joint.rotation_values);
        free(joint.scale_timers);
        free(joint.scale_values);
    }
    free(skin->joints);
}

/* joint updating/dumping */

void skin_update_joints(skin_t *skin, float timer) {
    int i, j;
    timer = (float)fmod(timer, skin->max_time);
    for (i = 0; i < skin->num_joints; i++) {
        joint_t *joint = &skin->joints[i];
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
            joint_t parent_joint = skin->joints[joint->parent_index];
            joint->transform = mat4_mul_mat4(parent_joint.transform,
                                             joint->transform);
        }
    }
}

void skin_dump_matrices(skin_t *skin, mat4_t matrices[256]) {
    int i;
    assert(skin->num_joints <= 256);
    for (i = 0; i < skin->num_joints; i++) {
        joint_t joint = skin->joints[i];
        matrices[i] = mat4_mul_mat4(joint.transform, joint.inverse_bind);
    }
}
