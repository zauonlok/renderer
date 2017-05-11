#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "model.h"
#include "buffer.h"
#include "error.h"
#include "geometry.h"

struct model {
    vec3f_t *vertices;
    vec2f_t *uvs;
    vec3f_t *normals;
    int num_faces;
};

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
        buffer_push(line, (char)c);
        if (c == '\n') {
            break;
        }
    }

    if (line == NULL) {
        return NULL;
    } else {
        buffer_push(line, '\0');
        return line;
    }
}

static model_t *build_model(vec3f_t *vertex_buffer, vec2f_t *uv_buffer,
                            vec3f_t *normal_buffer,
                            int *vertex_index_buffer, int *uv_index_buffer,
                            int *normal_index_buffer) {
    int num_vertices = buffer_size(vertex_buffer);
    int num_uvs = buffer_size(uv_buffer);
    int num_normals = buffer_size(normal_buffer);
    int num_faces = buffer_size(vertex_index_buffer) / 3;
    int num_face_vertices = buffer_size(vertex_index_buffer);
    model_t *model;
    int i;

    FORCE(num_faces > 0, "build_model: no face");

    model = (model_t*)malloc(sizeof(model_t));
    model->vertices = (vec3f_t*)malloc(num_face_vertices * sizeof(vec3f_t));
    model->uvs = (vec2f_t*)malloc(num_face_vertices * sizeof(vec2f_t));
    model->normals = (vec3f_t*)malloc(num_face_vertices * sizeof(vec3f_t));
    model->num_faces = num_faces;

    for (i = 0; i < num_face_vertices; i++) {
        int vertex_index = vertex_index_buffer[i];
        int uv_index = uv_index_buffer[i];
        int normal_index = normal_index_buffer[i];

        FORCE(vertex_index >= 0 && vertex_index < num_vertices,
              "build_model: vertex index range");
        FORCE(uv_index >= 0 && uv_index < num_uvs,
              "build_model: uv index range");
        FORCE(normal_index >= 0 && normal_index < num_normals,
              "build_model: normal index range");

        model->vertices[i] = vertex_buffer[vertex_index];
        model->uvs[i] = uv_buffer[uv_index];
        model->normals[i] = normal_buffer[normal_index];
    }

    return model;
}

static model_t *load_obj(const char *filename) {
    vec3f_t *vertex_buffer = NULL;
    vec2f_t *uv_buffer = NULL;
    vec3f_t *normal_buffer = NULL;
    int *vertex_index_buffer = NULL;
    int *uv_index_buffer = NULL;
    int *normal_index_buffer = NULL;
    model_t *model;
    FILE *file;

    file = fopen(filename, "rb");
    FORCE(file != NULL, "fopen");
    while (1) {
        char *line = read_line(file);
        char *curr = line;
        if (line == NULL) {
            break;
        }
        while (isspace(*curr)) {
            curr += 1;
        }
        if (strncmp(curr, "v ", 2) == 0) {          /* vertex */
            vec3f_t vertex;
            int items = sscanf(curr + 2, "%f %f %f",
                               &vertex.x, &vertex.y, &vertex.z);
            FORCE(items == 3, "load_obj: read v");
            buffer_push(vertex_buffer, vertex);
        } else if (strncmp(curr, "vt ", 3) == 0) {  /* texture */
            vec2f_t uv;
            int items = sscanf(curr + 3, "%f %f", &uv.x, &uv.y);
            FORCE(items == 2, "load_obj: read vt");
            buffer_push(uv_buffer, uv);
        } else if (strncmp(curr, "vn ", 3) == 0) {  /* normal */
            vec3f_t normal;
            int items = sscanf(curr + 3, "%f %f %f",
                               &normal.x, &normal.y, &normal.z);
            FORCE(items == 3, "load_obj: read vn");
            buffer_push(normal_buffer, normal);
        } else if (strncmp(curr, "f ", 2) == 0) {   /* face */
            int i;
            int vertex_indices[3], uv_indices[3], normal_indices[3];
            int items = sscanf(
                curr + 2, "%d/%d/%d %d/%d/%d %d/%d/%d",
                &vertex_indices[0], &uv_indices[0], &normal_indices[0],
                &vertex_indices[1], &uv_indices[1], &normal_indices[1],
                &vertex_indices[2], &uv_indices[2], &normal_indices[2]
            );
            FORCE(items == 9, "load_obj: read f");
            for (i = 0; i < 3; i++) {
                buffer_push(vertex_index_buffer, vertex_indices[i] - 1);
                buffer_push(uv_index_buffer, uv_indices[i] - 1);
                buffer_push(normal_index_buffer, normal_indices[i] - 1);
            }
        }
        buffer_free(line);
    }
    fclose(file);

    model = build_model(
        vertex_buffer, uv_buffer, normal_buffer,
        vertex_index_buffer, uv_index_buffer, normal_index_buffer
    );

    buffer_free(vertex_buffer);
    buffer_free(uv_buffer);
    buffer_free(normal_buffer);
    buffer_free(vertex_index_buffer);
    buffer_free(uv_index_buffer);
    buffer_free(normal_index_buffer);

    return model;
}

model_t *model_load(const char *filename) {
    const char *ext = extract_ext(filename);
    if (strcmp(ext, "obj") == 0) {
        return load_obj(filename);
    } else {
        FATAL("model_load: format");
        return NULL;
    }
}

void model_free(model_t *model) {
    free(model->vertices);
    free(model->uvs);
    free(model->normals);
    free(model);
}

int model_get_num_faces(model_t *model) {
    return model->num_faces;
}

static void check_range(model_t *model, int face, int nth_element) {
    FORCE(face >= 0 && face < model->num_faces, "check_range: face range");
    FORCE(nth_element >= 0 && nth_element < 3, "check_range: element range");
}

vec3f_t model_get_vertex(model_t *model, int face, int nth_vertex) {
    int index = face * 3 + nth_vertex;
    check_range(model, face, nth_vertex);
    return model->vertices[index];
}

vec2f_t model_get_uv(model_t *model, int face, int nth_uv) {
    int index = face * 3 + nth_uv;
    check_range(model, face, nth_uv);
    return model->uvs[index];
}

vec3f_t model_get_normal(model_t *model, int face, int nth_normal) {
    int index = face * 3 + nth_normal;
    check_range(model, face, nth_normal);
    return model->normals[index];
}
