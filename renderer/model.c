#include "model.h"
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

#define BUFFER_RAW_DATA(buffer) ((int*)(buffer) - 2)
#define BUFFER_CAPACITY(buffer) (BUFFER_RAW_DATA(buffer)[0])
#define BUFFER_OCCUPIED(buffer) (BUFFER_RAW_DATA(buffer)[1])

#define buffer_push(buffer, value)                                          \
    do {                                                                    \
        (buffer) = buffer_hold(buffer, 1, sizeof(*(buffer)));               \
        (buffer)[buffer_size(buffer) - 1] = (value);                        \
    } while (0)

static int buffer_size(void *buffer) {
    return buffer != NULL ? BUFFER_OCCUPIED(buffer) : 0;
}

static void buffer_free(void *buffer) {
    if (buffer != NULL) {
        free(BUFFER_RAW_DATA(buffer));
    }
}

static void *buffer_hold(void *buffer, int count, int itemsize) {
    if (buffer == NULL) {
        int *base = (int*)malloc(sizeof(int) * 2 + itemsize * count);
        base[0] = count;  /* capacity */
        base[1] = count;  /* occupied */
        return base + 2;
    } else if (BUFFER_OCCUPIED(buffer) + count <= BUFFER_CAPACITY(buffer)) {
        BUFFER_OCCUPIED(buffer) += count;
        return buffer;
    } else {
        int needed_size = BUFFER_OCCUPIED(buffer) + count;
        int double_curr = BUFFER_CAPACITY(buffer) * 2;
        int capacity = needed_size > double_curr ? needed_size : double_curr;
        int occupied = needed_size;
        int *base = (int*)realloc(BUFFER_RAW_DATA(buffer),
                                  sizeof(int) * 2 + itemsize * capacity);
        base[0] = capacity;
        base[1] = occupied;
        return base + 2;
    }
}

/* data structure */

struct model {
    vec3_t *positions;
    vec2_t *texcoords;
    vec3_t *normals;
    int num_faces;
};

/* model loading */

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

static model_t *build_model(vec3_t *position_buffer, int *position_index_buffer,
                            vec2_t *texcoord_buffer, int *texcoord_index_buffer,
                            vec3_t *normal_buffer, int *normal_index_buffer) {
    int num_positions = buffer_size(position_buffer);
    int num_texcoords = buffer_size(texcoord_buffer);
    int num_normals = buffer_size(normal_buffer);
    int num_faces = buffer_size(position_index_buffer) / 3;
    int num_indices = num_faces * 3;
    model_t *model;
    int i;

    assert(num_faces > 0);
    assert(num_indices == buffer_size(position_index_buffer));
    assert(num_indices == buffer_size(texcoord_index_buffer));
    assert(num_indices == buffer_size(normal_index_buffer));

    model = (model_t*)malloc(sizeof(model_t));
    model->positions = (vec3_t*)malloc(num_indices * sizeof(vec3_t));
    model->texcoords = (vec2_t*)malloc(num_indices * sizeof(vec2_t));
    model->normals   = (vec3_t*)malloc(num_indices * sizeof(vec3_t));
    model->num_faces = num_faces;

    for (i = 0; i < num_indices; i++) {
        int position_index = position_index_buffer[i];
        int texcoord_index = texcoord_index_buffer[i];
        int normal_index = normal_index_buffer[i];

        assert(position_index >= 0 && position_index < num_positions);
        assert(texcoord_index >= 0 && texcoord_index < num_texcoords);
        assert(normal_index >= 0 && normal_index < num_normals);

        model->positions[i] = position_buffer[position_index];
        model->texcoords[i] = texcoord_buffer[texcoord_index];
        model->normals[i] = normal_buffer[normal_index];
    }

    return model;
}

static model_t *load_obj(const char *filename) {
    vec3_t *position_buffer = NULL;
    vec2_t *texcoord_buffer = NULL;
    vec3_t *normal_buffer = NULL;
    int *position_index_buffer = NULL;
    int *texcoord_index_buffer = NULL;
    int *normal_index_buffer = NULL;
    model_t *model;
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
            buffer_push(position_buffer, position);
        } else if (strncmp(curr, "vt ", 3) == 0) {  /* texcoord */
            vec2_t texcoord;
            int items = sscanf(curr + 3, "%f %f", &texcoord.x, &texcoord.y);
            assert(items == 2);
            buffer_push(texcoord_buffer, texcoord);
        } else if (strncmp(curr, "vn ", 3) == 0) {  /* normal */
            vec3_t normal;
            int items = sscanf(curr + 3, "%f %f %f",
                               &normal.x, &normal.y, &normal.z);
            assert(items == 3);
            buffer_push(normal_buffer, normal);
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
                buffer_push(position_index_buffer, position_indices[i] - 1);
                buffer_push(texcoord_index_buffer, texcoord_indices[i] - 1);
                buffer_push(normal_index_buffer, normal_indices[i] - 1);
            }
        }
        buffer_free(line);
    }
    fclose(file);

    model = build_model(position_buffer, position_index_buffer,
                        texcoord_buffer, texcoord_index_buffer,
                        normal_buffer, normal_index_buffer);

    buffer_free(position_buffer);
    buffer_free(texcoord_buffer);
    buffer_free(normal_buffer);
    buffer_free(position_index_buffer);
    buffer_free(texcoord_index_buffer);
    buffer_free(normal_index_buffer);

    return model;
}

model_t *model_load(const char *filename) {
    const char *ext = extract_ext(filename);
    if (strcmp(ext, "obj") == 0) {
        return load_obj(filename);
    } else {
        assert(0);
        return NULL;
    }
}

void model_release(model_t *model) {
    free(model->positions);
    free(model->texcoords);
    free(model->normals);
    free(model);
}

/* vertex retrieving */

int model_get_num_faces(model_t *model) {
    return model->num_faces;
}

vec3_t model_get_position(model_t *model, int nth_face, int nth_position) {
    int index = nth_face * 3 + nth_position;
    assert(nth_face >= 0 && nth_face < model->num_faces);
    assert(nth_position >= 0 && nth_position < 3);
    return model->positions[index];
}

vec2_t model_get_texcoord(model_t *model, int nth_face, int nth_texcoord) {
    int index = nth_face * 3 + nth_texcoord;
    assert(nth_face >= 0 && nth_face < model->num_faces);
    assert(nth_texcoord >= 0 && nth_texcoord < 3);
    return model->texcoords[index];
}

vec3_t model_get_normal(model_t *model, int nth_face, int nth_normal) {
    int index = nth_face * 3 + nth_normal;
    assert(nth_face >= 0 && nth_face < model->num_faces);
    assert(nth_normal >= 0 && nth_normal < 3);
    return model->normals[index];
}
