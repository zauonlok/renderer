#include "mesh.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "geometry.h"

/*
 * typesafe dynamic array, see
 * https://github.com/nothings/stb/blob/master/stretchy_buffer.h
 */

#define DARRAY_RAW_DATA(darray) ((int*)(darray) - 2)
#define DARRAY_CAPACITY(darray) (DARRAY_RAW_DATA(darray)[0])
#define DARRAY_OCCUPIED(darray) (DARRAY_RAW_DATA(darray)[1])

#define darray_push(darray, value)                                          \
    do {                                                                    \
        (darray) = darray_hold(darray, 1, sizeof(*(darray)));               \
        (darray)[darray_size(darray) - 1] = (value);                        \
    } while (0)

static int darray_size(void *darray) {
    return darray != NULL ? DARRAY_OCCUPIED(darray) : 0;
}

static void darray_free(void *darray) {
    if (darray != NULL) {
        free(DARRAY_RAW_DATA(darray));
    }
}

static void *darray_hold(void *darray, int count, int itemsize) {
    if (darray == NULL) {
        int *base = (int*)malloc(sizeof(int) * 2 + itemsize * count);
        base[0] = count;  /* capacity */
        base[1] = count;  /* occupied */
        return base + 2;
    } else if (DARRAY_OCCUPIED(darray) + count <= DARRAY_CAPACITY(darray)) {
        DARRAY_OCCUPIED(darray) += count;
        return darray;
    } else {
        int needed_size = DARRAY_OCCUPIED(darray) + count;
        int double_curr = DARRAY_CAPACITY(darray) * 2;
        int capacity = needed_size > double_curr ? needed_size : double_curr;
        int occupied = needed_size;
        int size = sizeof(int) * 2 + itemsize * capacity;
        int *base = (int*)realloc(DARRAY_RAW_DATA(darray), size);
        base[0] = capacity;
        base[1] = occupied;
        return base + 2;
    }
}

/* data structure */

struct mesh {
    vec3_t *positions;
    vec2_t *texcoords;
    vec3_t *normals;
    int num_faces;
};

/* mesh loading/releasing */

static const char *extract_ext(const char *filename) {
    const char *dot_pos = strrchr(filename, '.');
    return (dot_pos == NULL) ? "" : dot_pos + 1;
}

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
                          vec3_t *normal_darray, int *normal_index_darray) {
    int num_indices = darray_size(position_index_darray);
    int num_faces = num_indices / 3;
    mesh_t *mesh;
    int i;

    assert(darray_size(position_index_darray) == num_indices);
    assert(darray_size(texcoord_index_darray) == num_indices);
    assert(darray_size(normal_index_darray) == num_indices);
    assert(num_faces > 0);

    mesh = (mesh_t*)malloc(sizeof(mesh_t));
    mesh->positions = (vec3_t*)malloc(num_indices * sizeof(vec3_t));
    mesh->texcoords = (vec2_t*)malloc(num_indices * sizeof(vec2_t));
    mesh->normals   = (vec3_t*)malloc(num_indices * sizeof(vec3_t));
    mesh->num_faces = num_faces;

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

    return mesh;
}

static mesh_t *load_obj(const char *filename) {
    vec3_t *position_darray = NULL;
    vec2_t *texcoord_darray = NULL;
    vec3_t *normal_darray = NULL;
    int *position_index_darray = NULL;
    int *texcoord_index_darray = NULL;
    int *normal_index_darray = NULL;
    mesh_t *mesh;
    FILE *file;

    file = fopen(filename, "rb");
    assert(file != NULL);
    while (1) {
        char *line = read_line(file);
        char *curr = line;
        if (line == NULL) {
            break;
        }
        while (isspace(*curr)) {
            curr += 1;
        }
        if (strncmp(curr, "v ", 2) == 0) {          /* position */
            vec3_t position;
            int items = sscanf(curr + 2, "%f %f %f",
                               &position.x, &position.y, &position.z);
            assert(items == 3);
            darray_push(position_darray, position);
        } else if (strncmp(curr, "vt ", 3) == 0) {  /* texcoord */
            vec2_t texcoord;
            int items = sscanf(curr + 3, "%f %f", &texcoord.x, &texcoord.y);
            assert(items == 2);
            darray_push(texcoord_darray, texcoord);
        } else if (strncmp(curr, "vn ", 3) == 0) {  /* normal */
            vec3_t normal;
            int items = sscanf(curr + 3, "%f %f %f",
                               &normal.x, &normal.y, &normal.z);
            assert(items == 3);
            darray_push(normal_darray, normal);
        } else if (strncmp(curr, "f ", 2) == 0) {   /* face */
            int i;
            int position_indices[3], texcoord_indices[3], normal_indices[3];
            int items = sscanf(
                curr + 2, "%d/%d/%d %d/%d/%d %d/%d/%d",
                &position_indices[0], &texcoord_indices[0], &normal_indices[0],
                &position_indices[1], &texcoord_indices[1], &normal_indices[1],
                &position_indices[2], &texcoord_indices[2], &normal_indices[2]);
            assert(items == 9);
            for (i = 0; i < 3; i++) {
                darray_push(position_index_darray, position_indices[i] - 1);
                darray_push(texcoord_index_darray, texcoord_indices[i] - 1);
                darray_push(normal_index_darray, normal_indices[i] - 1);
            }
        }
        darray_free(line);
    }
    fclose(file);

    mesh = build_mesh(position_darray, position_index_darray,
                      texcoord_darray, texcoord_index_darray,
                      normal_darray, normal_index_darray);

    darray_free(position_darray);
    darray_free(texcoord_darray);
    darray_free(normal_darray);
    darray_free(position_index_darray);
    darray_free(texcoord_index_darray);
    darray_free(normal_index_darray);

    return mesh;
}

mesh_t *mesh_load(const char *filename) {
    const char *ext = extract_ext(filename);
    if (strcmp(ext, "obj") == 0) {
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
    free(mesh);
}

/* vertex retrieving */

int mesh_get_num_faces(mesh_t *mesh) {
    return mesh->num_faces;
}

vec3_t mesh_get_position(mesh_t *mesh, int nth_face, int nth_position) {
    int index = nth_face * 3 + nth_position;
    assert(nth_face >= 0 && nth_face < mesh->num_faces);
    assert(nth_position >= 0 && nth_position < 3);
    return mesh->positions[index];
}

vec2_t mesh_get_texcoord(mesh_t *mesh, int nth_face, int nth_texcoord) {
    int index = nth_face * 3 + nth_texcoord;
    assert(nth_face >= 0 && nth_face < mesh->num_faces);
    assert(nth_texcoord >= 0 && nth_texcoord < 3);
    return mesh->texcoords[index];
}

vec3_t mesh_get_normal(mesh_t *mesh, int nth_face, int nth_normal) {
    int index = nth_face * 3 + nth_normal;
    assert(nth_face >= 0 && nth_face < mesh->num_faces);
    assert(nth_normal >= 0 && nth_normal < 3);
    return mesh->normals[index];
}
