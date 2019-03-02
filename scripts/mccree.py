"""Preprocess the McCree model

The model is available for download from
    https://sketchfab.com/models/38aedc02c0b2412babdc4d0eac7c6803
"""

from __future__ import print_function

import json
import os
import zipfile
from utils.gltf import dump_obj_data, load_node_data

SRC_FILENAME = "low_poly_mccree.zip"
DST_DIRECTORY = "../assets/mccree"


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    for mesh_index in range(len(gltf["meshes"])):
        obj_data = dump_obj_data(gltf, buffer, mesh_index)
        filename = "mccree{}.obj".format(mesh_index)
        filepath = os.path.join(DST_DIRECTORY, filename)
        with open(filepath, "w") as f:
            f.write(obj_data)


def linear_to_srgb(color):
    red, green, blue = [pow(c, 1 / 2.2) for c in color[:3]]
    return red, green, blue


def print_transforms(transforms):
    row_pattern = "            {{{:+.6f}f, {:+.6f}f, {:+.6f}f, {:+.6f}f}},"
    print("    mat4_t transforms[{}] = {{".format(len(transforms)))
    for transform in transforms:
        print("        {{")
        for i in range(4):
            print(row_pattern.format(*transform.data[i]))
        print("        }},")
    print("    };")


def print_materials(materials):
    row_pattern = "        {{{{{:.3f}f, {:.3f}f, {:.3f}f, 1}}, NULL, 1, 0}},"
    print("    unlit_material_t materials[{}] = {{".format(len(materials)))
    for material in materials:
        color = material["pbrMetallicRoughness"]["baseColorFactor"]
        print(row_pattern.format(*linear_to_srgb(color)))
    print("    };")


def print_mesh2transform(nodes, transforms):
    mesh2transform = []
    for node in nodes:
        index = transforms.index(node.world_transform)
        mesh2transform.append(index)

    num_meshes = len(mesh2transform)
    chunk_size = num_meshes / 3 + 1
    print("    int mesh2transform[{}] = {{".format(num_meshes))
    for i in range(0, num_meshes, chunk_size):
        indices = [str(j) for j in mesh2transform[i:(i + chunk_size)]]
        print("        {}".format(", ".join(indices) + ","))
    print("    };")


def print_mesh2material(meshes):
    mesh2material = []
    for mesh in meshes:
        assert len(mesh["primitives"]) == 1
        primitive = mesh["primitives"][0]
        mesh2material.append(primitive["material"])

    num_meshes = len(mesh2material)
    chunk_size = num_meshes / 3 + 1
    print("    int mesh2material[{}] = {{".format(num_meshes))
    for i in range(0, num_meshes, chunk_size):
        indices = [format(j, "2d") for j in mesh2material[i:(i + chunk_size)]]
        print("        {}".format(", ".join(indices) + ","))
    print("    };")


def generated_code(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    nodes = load_node_data(gltf)
    nodes = [node for node in nodes if node.mesh is not None]

    transforms = []
    for node in nodes:
        if node.world_transform not in transforms:
            transforms.append(node.world_transform)

    print_transforms(transforms)
    print_materials(gltf["materials"])
    print_mesh2transform(nodes, transforms)
    print_mesh2material(gltf["meshes"])


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        # generated_code(zip_file)


if __name__ == "__main__":
    main()
