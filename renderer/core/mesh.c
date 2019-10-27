#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "darray.h"
#include "macro.h"
#include "maths.h"
#include "mesh.h"
#include "private.h"

struct mesh {
    int num_faces;
    vertex_t *vertices;
    vec3_t center;
};

/* mesh loading/releasing */

static mesh_t *build_mesh(
        vec3_t *positions, vec2_t *texcoords, vec3_t *normals,
        vec4_t *tangents, vec4_t *joints, vec4_t *weights,
        int *position_indices, int *texcoord_indices, int *normal_indices) {
    vec3_t bbox_min = vec3_new(+1e6, +1e6, +1e6);
    vec3_t bbox_max = vec3_new(-1e6, -1e6, -1e6);
    int num_indices = darray_size(position_indices);
    int num_faces = num_indices / 3;
    vertex_t *vertices;
    mesh_t *mesh;
    int i;

    assert(num_faces > 0 && num_faces * 3 == num_indices);
    assert(darray_size(position_indices) == num_indices);
    assert(darray_size(texcoord_indices) == num_indices);
    assert(darray_size(normal_indices) == num_indices);

    vertices = (vertex_t*)malloc(sizeof(vertex_t) * num_indices);
    for (i = 0; i < num_indices; i++) {
        int position_index = position_indices[i];
        int texcoord_index = texcoord_indices[i];
        int normal_index = normal_indices[i];
        assert(position_index >= 0 && position_index < darray_size(positions));
        assert(texcoord_index >= 0 && texcoord_index < darray_size(texcoords));
        assert(normal_index >= 0 && normal_index < darray_size(normals));
        vertices[i].position = positions[position_index];
        vertices[i].texcoord = texcoords[texcoord_index];
        vertices[i].normal = normals[normal_index];

        if (tangents) {
            int tangent_index = position_index;
            assert(tangent_index >= 0 && tangent_index < darray_size(tangents));
            vertices[i].tangent = tangents[tangent_index];
        } else {
            vertices[i].tangent = vec4_new(1, 0, 0, 1);
        }

        if (joints) {
            int joint_index = position_index;
            assert(joint_index >= 0 && joint_index < darray_size(joints));
            vertices[i].joint = joints[joint_index];
        } else {
            vertices[i].joint = vec4_new(0, 0, 0, 0);
        }

        if (weights) {
            int weight_index = position_index;
            assert(weight_index >= 0 && weight_index < darray_size(weights));
            vertices[i].weight = weights[weight_index];
        } else {
            vertices[i].weight = vec4_new(0, 0, 0, 0);
        }

        bbox_min = vec3_min(bbox_min, vertices[i].position);
        bbox_max = vec3_max(bbox_max, vertices[i].position);
    }

    mesh = (mesh_t*)malloc(sizeof(mesh_t));
    mesh->num_faces = num_faces;
    mesh->vertices = vertices;
    mesh->center = vec3_div(vec3_add(bbox_min, bbox_max), 2);

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
    char line[LINE_SIZE];
    mesh_t *mesh;
    FILE *file;

    file = fopen(filename, "rb");
    assert(file != NULL);
    while (1) {
        int items;
        if (fgets(line, LINE_SIZE, file) == NULL) {
            break;
        } else if (strncmp(line, "v ", 2) == 0) {               /* position */
            vec3_t position;
            items = sscanf(line, "v %f %f %f",
                           &position.x, &position.y, &position.z);
            assert(items == 3);
            darray_push(positions, position);
        } else if (strncmp(line, "vt ", 3) == 0) {              /* texcoord */
            vec2_t texcoord;
            items = sscanf(line, "vt %f %f",
                           &texcoord.x, &texcoord.y);
            assert(items == 2);
            darray_push(texcoords, texcoord);
        } else if (strncmp(line, "vn ", 3) == 0) {              /* normal */
            vec3_t normal;
            items = sscanf(line, "vn %f %f %f",
                           &normal.x, &normal.y, &normal.z);
            assert(items == 3);
            darray_push(normals, normal);
        } else if (strncmp(line, "f ", 2) == 0) {               /* face */
            int i;
            int pos_indices[3], uv_indices[3], n_indices[3];
            items = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                           &pos_indices[0], &uv_indices[0], &n_indices[0],
                           &pos_indices[1], &uv_indices[1], &n_indices[1],
                           &pos_indices[2], &uv_indices[2], &n_indices[2]);
            assert(items == 9);
            for (i = 0; i < 3; i++) {
                darray_push(position_indices, pos_indices[i] - 1);
                darray_push(texcoord_indices, uv_indices[i] - 1);
                darray_push(normal_indices, n_indices[i] - 1);
            }
        } else if (strncmp(line, "# ext.tangent ", 14) == 0) {  /* tangent */
            vec4_t tangent;
            items = sscanf(line, "# ext.tangent %f %f %f %f",
                           &tangent.x, &tangent.y, &tangent.z, &tangent.w);
            assert(items == 4);
            darray_push(tangents, tangent);
        } else if (strncmp(line, "# ext.joint ", 12) == 0) {    /* joint */
            vec4_t joint;
            items = sscanf(line, "# ext.joint %f %f %f %f",
                           &joint.x, &joint.y, &joint.z, &joint.w);
            assert(items == 4);
            darray_push(joints, joint);
        } else if (strncmp(line, "# ext.weight ", 13) == 0) {   /* weight */
            vec4_t weight;
            items = sscanf(line, "# ext.weight %f %f %f %f",
                           &weight.x, &weight.y, &weight.z, &weight.w);
            assert(items == 4);
            darray_push(weights, weight);
        }
        UNUSED_VAR(items);
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

mesh_t *mesh_load(const char *filename) {
    const char *extension = private_get_extension(filename);
    if (strcmp(extension, "obj") == 0) {
        return load_obj(filename);
    } else {
        assert(0);
        return NULL;
    }
}

void mesh_release(mesh_t *mesh) {
    free(mesh->vertices);
    free(mesh);
}

/* vertex retrieving */

int mesh_get_num_faces(mesh_t *mesh) {
    return mesh->num_faces;
}

vertex_t *mesh_get_vertices(mesh_t *mesh) {
    return mesh->vertices;
}

vec3_t mesh_get_center(mesh_t *mesh) {
    return mesh->center;
}
