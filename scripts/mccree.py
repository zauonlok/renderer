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


def print_generated_code(gltf):
    num_materials = len(gltf["materials"])
    print("    constant_material_t materials[{}] = {{".format(num_materials))
    for material in gltf["materials"]:
        ambient = material["pbrMetallicRoughness"]["baseColorFactor"]
        amb_r, amb_g, amb_b, _ = ambient
        amb_pattern = "{{{:.3f}f, {:.3f}f, {:.3f}f, 1}}"
        amb_content = amb_pattern.format(amb_r, amb_g, amb_b)
        emissive = material.get("emissiveFactor", [0, 0, 0])
        emi_r, emi_g, emi_b = emissive
        emi_pattern = "{{{:.3f}f, {:.3f}f, {:.3f}f, 1}}"
        emi_content = emi_pattern.format(emi_r, emi_g, emi_b)
        print("        {{{}, {}, NULL}},".format(amb_content, emi_content))
    print("    };")

    mesh2material = []
    for mesh in gltf["meshes"]:
        assert len(mesh["primitives"]) == 1
        primitive = mesh["primitives"][0]
        mesh2material.append(primitive["material"])
    num_meshes = len(gltf["meshes"])
    print("    int mesh2material[{}] = {{".format(num_meshes))
    chunk_size = num_meshes / 3 + 1
    for i in range(0, num_meshes, chunk_size):
        indices = [str(j) for j in mesh2material[i:(i + chunk_size)]]
        print("        {}".format(", ".join(indices) + ","))
    print("    };")

    transforms = utils.load_gltf_transforms(gltf)
    num_transforms = len(transforms)
    print("    mat4_t transforms[{}] = {{".format(num_transforms))
    for transform in transforms:
        print("        {{")
        for i in range(4):
            m0, m1, m2, m3 = transform.data[i]
            row_pattern = "            {{{:.6f}f, {:.6f}f, {:.6f}f, {:.6f}f}},"
            row_content = row_pattern.format(m0, m1, m2, m3)
            print(row_content)
        print("        }},")
    print("    };")

    nodes = utils.load_gltf_nodes(gltf)
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


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")
    assert len(buffer) == 299192

    meshes = []
    for mesh in gltf["meshes"]:
        mesh = utils.load_gltf_mesh(gltf, buffer, mesh)
        meshes.append(mesh)

    for index, mesh in enumerate(meshes):
        content = utils.dump_mesh_data(mesh)
        filename = "part{}.obj".format(index)
        filepath = os.path.join(DST_DIRECTORY, filename)
        with open(filepath, "w") as f:
            f.write(content)

    print_generated_code(gltf)


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)


if __name__ == "__main__":
    main()
