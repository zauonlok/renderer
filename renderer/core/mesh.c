#include <assert.h>
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
    float *translation_indices;
    vec3_t *translation_values;
    /* rotations */
    int num_rotations;
    float *rotation_indices;
    quat_t *rotation_values;
    /* scales */
    int num_scales;
    float *scale_indices;
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

static mesh_t *build_mesh(vec3_t *position_darray, int *position_index_darray,
                          vec2_t *texcoord_darray, int *texcoord_index_darray,
                          vec3_t *normal_darray, int *normal_index_darray,
                          vec4_t *tangent_darray, vec4_t *joint_darray,
                          vec4_t *weight_darray) {
    int num_indices = darray_size(position_index_darray);
    int num_faces = num_indices / 3;
    mesh_t *mesh;
    int i;

    assert(num_faces > 0 && num_faces * 3 == num_indices);
    assert(darray_size(position_index_darray) == num_indices);
    assert(darray_size(texcoord_index_darray) == num_indices);
    assert(darray_size(normal_index_darray) == num_indices);

    mesh = (mesh_t*)malloc(sizeof(mesh_t));
    mesh->num_faces = num_faces;
    mesh->positions = (vec3_t*)malloc(sizeof(vec3_t) * num_indices);
    mesh->texcoords = (vec2_t*)malloc(sizeof(vec2_t) * num_indices);
    mesh->normals   = (vec3_t*)malloc(sizeof(vec3_t) * num_indices);
    mesh->tangents  = NULL;
    mesh->joints    = NULL;
    mesh->weights   = NULL;

    for (i = 0; i < num_indices; i++) {
        int position_index = position_index_darray[i];
        int texcoord_index = texcoord_index_darray[i];
        int normal_index = normal_index_darray[i];
        assert(position_index >= 0);
        assert(position_index < darray_size(position_darray));
        assert(texcoord_index >= 0);
        assert(texcoord_index < darray_size(texcoord_darray));
        assert(normal_index >= 0);
        assert(normal_index < darray_size(normal_darray));
        mesh->positions[i] = position_darray[position_index];
        mesh->texcoords[i] = texcoord_darray[texcoord_index];
        mesh->normals[i] = normal_darray[normal_index];
    }

    if (tangent_darray) {
        mesh->tangents = (vec4_t*)malloc(sizeof(vec4_t) * num_indices);
        for (i = 0; i < num_indices; i++) {
            int tangent_index = position_index_darray[i];
            assert(tangent_index >= 0);
            assert(tangent_index < darray_size(tangent_darray));
            mesh->tangents[i] = tangent_darray[tangent_index];
        }
    }

    if (joint_darray) {
        mesh->joints = (vec4_t*)malloc(sizeof(vec4_t) * num_indices);
        for (i = 0; i < num_indices; i++) {
            int joint_index = position_index_darray[i];
            assert(joint_index >= 0);
            assert(joint_index < darray_size(joint_darray));
            mesh->joints[i] = joint_darray[joint_index];
        }
    }

    if (weight_darray) {
        mesh->weights = (vec4_t*)malloc(sizeof(vec4_t) * num_indices);
        for (i = 0; i < num_indices; i++) {
            int weight_index = position_index_darray[i];
            assert(weight_index >= 0);
            assert(weight_index < darray_size(weight_darray));
            mesh->weights[i] = weight_darray[weight_index];
        }
    }

    return mesh;
}

static mesh_t *load_obj(const char *filename) {
    vec3_t *position_darray = NULL;
    vec2_t *texcoord_darray = NULL;
    vec3_t *normal_darray = NULL;
    int *position_index_darray = NULL;
    int *texcoord_index_darray = NULL;
    int *normal_index_darray = NULL;
    vec4_t *tangent_darray = NULL;
    vec4_t *joint_darray = NULL;
    vec4_t *weight_darray = NULL;
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
            darray_push(position_darray, position);
        } else if (strncmp(line, "vt ", 3) == 0) {              /* texcoord */
            vec2_t texcoord;
            items = sscanf(line + 3, "%f %f", &texcoord.x, &texcoord.y);
            assert(items == 2);
            darray_push(texcoord_darray, texcoord);
        } else if (strncmp(line, "vn ", 3) == 0) {              /* normal */
            vec3_t normal;
            items = sscanf(line + 3, "%f %f %f",
                           &normal.x, &normal.y, &normal.z);
            assert(items == 3);
            darray_push(normal_darray, normal);
        } else if (strncmp(line, "f ", 2) == 0) {               /* face */
            int i;
            int position_indices[3], texcoord_indices[3], normal_indices[3];
            items = sscanf(
                line + 2, "%d/%d/%d %d/%d/%d %d/%d/%d",
                &position_indices[0], &texcoord_indices[0], &normal_indices[0],
                &position_indices[1], &texcoord_indices[1], &normal_indices[1],
                &position_indices[2], &texcoord_indices[2], &normal_indices[2]);
            assert(items == 9);
            for (i = 0; i < 3; i++) {
                darray_push(position_index_darray, position_indices[i] - 1);
                darray_push(texcoord_index_darray, texcoord_indices[i] - 1);
                darray_push(normal_index_darray, normal_indices[i] - 1);
            }
        } else if (strncmp(line, "# ext.tangent ", 14) == 0) {  /* tangent */
            vec4_t tangent;
            items = sscanf(line + 14, "%f %f %f %f",
                           &tangent.x, &tangent.y, &tangent.z, &tangent.w);
            assert(items == 4);
            darray_push(tangent_darray, tangent);
        } else if (strncmp(line, "# ext.joint ", 12) == 0) {    /* joint */
            vec4_t joint;
            items = sscanf(line + 12, "%f %f %f %f",
                           &joint.x, &joint.y, &joint.z, &joint.w);
            assert(items == 4);
            darray_push(joint_darray, joint);
        } else if (strncmp(line, "# ext.weight ", 13) == 0) {   /* weight */
            vec4_t weight;
            items = sscanf(line + 13, "%f %f %f %f",
                           &weight.x, &weight.y, &weight.z, &weight.w);
            assert(items == 4);
            darray_push(weight_darray, weight);
        }
        darray_free(line);
    }
    fclose(file);

    mesh = build_mesh(position_darray, position_index_darray,
                      texcoord_darray, texcoord_index_darray,
                      normal_darray, normal_index_darray,
                      tangent_darray, joint_darray, weight_darray);
    darray_free(position_darray);
    darray_free(texcoord_darray);
    darray_free(normal_darray);
    darray_free(position_index_darray);
    darray_free(texcoord_index_darray);
    darray_free(normal_index_darray);
    darray_free(tangent_darray);
    darray_free(joint_darray);
    darray_free(weight_darray);

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
        if (strlen(line) == 0 || line[0] == '\r') {
            darray_free(line);
        } else {
            break;
        }
    }
    items = sscanf(line, "joint %d:", &dummy);
    assert(items == 1);
    darray_free(line);

    line = expect_line(file);
    items = sscanf(line, "parent-index: %d", &joint.parent_index);
    assert(items == 1);
    darray_free(line);

    line = expect_line(file);
    assert(strstr(line, "inverse-bind:") != NULL);
    darray_free(line);
    for (i = 0; i < 4; i++) {
        line = expect_line(file);
        items = sscanf(
            line, "%f %f %f %f",
            &joint.inverse_bind.m[i][0], &joint.inverse_bind.m[i][1],
            &joint.inverse_bind.m[i][2], &joint.inverse_bind.m[i][3]);
        assert(items == 4);
        darray_free(line);
    }

    line = expect_line(file);
    items = sscanf(line, "translations %d:", &joint.num_translations);
    assert(items == 1 && joint.num_translations >= 0);
    darray_free(line);
    if (joint.num_translations > 0) {
        int index_size = (int)sizeof(float) * joint.num_translations;
        int value_size = (int)sizeof(vec3_t) * joint.num_translations;
        joint.translation_indices = (float*)malloc(index_size);
        joint.translation_values = (vec3_t*)malloc(value_size);
        for (i = 0; i < joint.num_translations; i++) {
            line = expect_line(file);
            items = sscanf(line, "time: %f, value: [%f, %f, %f]",
                           &joint.translation_indices[i],
                           &joint.translation_values[i].x,
                           &joint.translation_values[i].y,
                           &joint.translation_values[i].z);
            assert(items == 4);
            darray_free(line);
        }
    } else {
        joint.translation_indices = NULL;
        joint.translation_values = NULL;
    }

    line = expect_line(file);
    items = sscanf(line, "rotations %d:", &joint.num_rotations);
    assert(items == 1 && joint.num_rotations >= 0);
    darray_free(line);
    if (joint.num_rotations > 0) {
        int index_size = (int)sizeof(float) * joint.num_rotations;
        int value_size = (int)sizeof(quat_t) * joint.num_rotations;
        joint.rotation_indices = (float*)malloc(index_size);
        joint.rotation_values = (quat_t*)malloc(value_size);
        for (i = 0; i < joint.num_rotations; i++) {
            line = expect_line(file);
            items = sscanf(line, "time: %f, value: [%f, %f, %f, %f]",
                           &joint.rotation_indices[i],
                           &joint.rotation_values[i].x,
                           &joint.rotation_values[i].y,
                           &joint.rotation_values[i].z,
                           &joint.rotation_values[i].w);
            assert(items == 4);
            darray_free(line);
        }
    } else {
        joint.rotation_indices = NULL;
        joint.rotation_values = NULL;
    }

    line = expect_line(file);
    items = sscanf(line, "scales %d:", &joint.num_scales);
    assert(items == 1 && joint.num_rotations >= 0);
    darray_free(line);
    if (joint.num_rotations > 0) {
        int index_size = (int)sizeof(float) * joint.num_scales;
        int value_size = (int)sizeof(vec3_t) * joint.num_scales;
        joint.scale_indices = (float*)malloc(index_size);
        joint.scale_values = (vec3_t*)malloc(value_size);
        for (i = 0; i < joint.num_scales; i++) {
            line = expect_line(file);
            items = sscanf(line, "time: %f, value: [%f, %f, %f]",
                           &joint.scale_indices[i],
                           &joint.scale_values[i].x,
                           &joint.scale_values[i].y,
                           &joint.scale_values[i].z);
            assert(items == 4);
            darray_free(line);
        }
    } else {
        joint.scale_indices = NULL;
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

    file = fopen(filename, "rb");
    assert(file != NULL);

    skin = (skin_t*)malloc(sizeof(skin_t));

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
        free(joint.translation_indices);
        free(joint.translation_values);
        free(joint.rotation_indices);
        free(joint.rotation_values);
        free(joint.scale_indices);
        free(joint.scale_values);
    }
    free(skin->joints);
}
