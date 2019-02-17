"""Preprocess the McCree model

The model is available for download from
    https://sketchfab.com/models/38aedc02c0b2412babdc4d0eac7c6803
"""

from __future__ import print_function

import json
import os
import zipfile
import utils

SRC_FILENAME = "low_poly_mccree.zip"
DST_DIRECTORY = "../assets/mccree"


def linear_to_srgb(color):
    red, green, blue = [pow(c, 1 / 2.2) for c in color[:3]]
    return red, green, blue


def print_transforms(gltf):
    row_pattern = "            {{{:+.6f}f, {:+.6f}f, {:+.6f}f, {:+.6f}f}},"
    transforms = utils.load_gltf_transforms(gltf)
    num_transforms = len(transforms)
    print("    mat4_t transforms[{}] = {{".format(num_transforms))
    for transform in transforms:
        print("        {{")
        for i in range(4):
            print(row_pattern.format(*transform.data[i]))
        print("        }},")
    print("    };")


def print_materials(gltf):
    row_pattern = "        {{{{{:.3f}f, {:.3f}f, {:.3f}f}}, NULL}},"
    num_materials = len(gltf["materials"])
    print("    unlit_material_t materials[{}] = {{".format(num_materials))
    for material in gltf["materials"]:
        color = material["pbrMetallicRoughness"]["baseColorFactor"]
        print(row_pattern.format(*linear_to_srgb(color)))
    print("    };")


def print_mesh2transform(gltf):
    num_meshes = len(gltf["meshes"])
    nodes = utils.load_gltf_nodes(gltf)
    transforms = utils.load_gltf_transforms(gltf)
    nodes = [node for node in nodes if node.mesh is not None]
    mesh2transform = [None] * num_meshes
    for node in nodes:
        for i, transform in enumerate(transforms):
            if node.world_transform.data == transform.data:
                assert mesh2transform[node.mesh] is None
                mesh2transform[node.mesh] = i
    print("    int mesh2transform[{}] = {{".format(num_meshes))
    chunk_size = num_meshes / 3 + 1
    for i in range(0, num_meshes, chunk_size):
        indices = [str(j) for j in mesh2transform[i:(i + chunk_size)]]
        print("        {}".format(", ".join(indices) + ","))
    print("    };")


def print_mesh2material(gltf):
    mesh2material = []
    for mesh in gltf["meshes"]:
        assert len(mesh["primitives"]) == 1
        primitive = mesh["primitives"][0]
        mesh2material.append(primitive["material"])
    num_meshes = len(gltf["meshes"])
    print("    int mesh2material[{}] = {{".format(num_meshes))
    chunk_size = num_meshes / 3 + 1
    for i in range(0, num_meshes, chunk_size):
        indices = []
        for j in mesh2material[i:(i + chunk_size)]:
            indices.append("{:2d}".format(j))
        indices = ", ".join(indices) + ","
        print("        {}".format(indices))
    print("    };")


def print_generated_code(gltf):
    print_transforms(gltf)
    print_materials(gltf)
    print_mesh2transform(gltf)
    print_mesh2material(gltf)


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    meshes = []
    for mesh in gltf["meshes"]:
        mesh = utils.load_gltf_mesh(gltf, buffer, mesh)
        meshes.append(mesh)

    for index, mesh in enumerate(meshes):
        obj_data, _ = utils.dump_mesh_data(mesh)
        filename = "part{}.obj".format(index)
        filepath = os.path.join(DST_DIRECTORY, filename)
        with open(filepath, "w") as f:
            f.write(obj_data)

    print_generated_code(gltf)


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)


if __name__ == "__main__":
    main()
