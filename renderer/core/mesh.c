#include "mesh.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "darray.h"
#include "geometry.h"

struct mesh {
    vec3_t *positions;
    vec2_t *texcoords;
    vec3_t *normals;
    int num_faces;
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
                          vec3_t *normal_darray, int *normal_index_darray) {
    int num_indices = darray_size(position_index_darray);
    int num_faces = num_indices / 3;
    mesh_t *mesh;
    int i;

    assert(num_faces > 0 && num_faces * 3 == num_indices);
    assert(darray_size(position_index_darray) == num_indices);
    assert(darray_size(texcoord_index_darray) == num_indices);
    assert(darray_size(normal_index_darray) == num_indices);

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

static const char *extract_ext(const char *filename) {
    const char *dot_pos = strrchr(filename, '.');
    return (dot_pos == NULL) ? "" : dot_pos + 1;
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

static void validate_vertex(mesh_t *mesh, int nth_face, int nth_vertex) {
    assert(nth_face >= 0 && nth_face < mesh->num_faces);
    assert(nth_vertex >= 0 && nth_vertex < 3);
}

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
