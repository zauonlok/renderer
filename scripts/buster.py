"""Preprocess the Buster Drone model

The model is available for download from
    https://sketchfab.com/models/294e79652f494130ad2ab00a13fdbafd

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile

from PIL import Image

from utils.gltf import dump_node_ani_data, dump_obj_data

SRC_FILENAME = "buster_drone.zip"
DST_DIRECTORY = "../assets/buster"

IMG_FILENAMES = {
    "boden": [
        "textures/Boden_baseColor.png",
        "textures/Boden_metallicRoughness.png",
        None,
    ],
    "body": [
        "textures/body_baseColor.png",
        "textures/body_metallicRoughness.png",
        "textures/body_emissive.png",
    ],
    "legs": [
        "textures/material_baseColor.png",
        "textures/material_metallicRoughness.png",
        None,
    ],
}

DEL_FILENAMES = [
    "boden_metalness.tga",
    "boden_occlusion.tga",
]


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    for mesh_index in range(len(gltf["meshes"])):
        obj_data = dump_obj_data(gltf, buffer, mesh_index)
        obj_filename = "buster{}.obj".format(mesh_index)
        obj_filepath = os.path.join(DST_DIRECTORY, obj_filename)
        with open(obj_filepath, "w") as f:
            f.write(obj_data)

    ani_data, _ = dump_node_ani_data(gltf, buffer)
    ani_filepath = os.path.join(DST_DIRECTORY, "buster.ani")
    with open(ani_filepath, "w") as f:
        f.write(ani_data)


def load_image(zip_file, filename):
    with zip_file.open(filename) as f:
        image = Image.open(f)
        image = image.transpose(Image.FLIP_TOP_BOTTOM)
        return image


def save_image(image, filename):
    image = image.resize((512, 512), Image.LANCZOS)
    filepath = os.path.join(DST_DIRECTORY, filename)
    image.save(filepath, rle=True)


def process_images(zip_file):
    for name, paths in IMG_FILENAMES.items():
        basecolor_path, packed_path, emission_path = paths

        if basecolor_path:
            basecolor_image = load_image(zip_file, basecolor_path)
            save_image(basecolor_image, "{}_basecolor.tga".format(name))

        if packed_path:
            packed_image = load_image(zip_file, packed_path)
            packed_bands = packed_image.split()
            occlusion_image, roughness_image, metalness_image = packed_bands
            save_image(occlusion_image, "{}_occlusion.tga".format(name))
            save_image(roughness_image, "{}_roughness.tga".format(name))
            save_image(metalness_image, "{}_metalness.tga".format(name))

        if emission_path:
            emission_image = load_image(zip_file, emission_path)
            save_image(emission_image, "{}_emission.tga".format(name))

    for del_filename in DEL_FILENAMES:
        del_filepath = os.path.join(DST_DIRECTORY, del_filename)
        os.remove(del_filepath)


def print_mesh2node(gltf, buffer):
    _, mesh2node = dump_node_ani_data(gltf, buffer)
    num_meshes = len(mesh2node)
    chunk_size = (num_meshes + 2) / 3
    print("    int mesh2node[{}] = {{".format(num_meshes))
    for i in range(0, num_meshes, chunk_size):
        indices = [str(j) for j in mesh2node[i:(i + chunk_size)]]
        print("        {}".format(", ".join(indices) + ","))
    print("    };")


def print_mesh2material(gltf):
    mesh2material = []
    meshes = gltf["meshes"]
    for mesh in meshes:
        assert len(mesh["primitives"]) == 1
        primitive = mesh["primitives"][0]
        mesh2material.append(primitive["material"])

    num_meshes = len(meshes)
    chunk_size = (num_meshes + 1) / 2
    print("    int mesh2material[{}] = {{".format(num_meshes))
    for i in range(0, num_meshes, chunk_size):
        indices = [str(j) for j in mesh2material[i:(i + chunk_size)]]
        print("        {}".format(", ".join(indices) + ","))
    print("    };")


def generated_code(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")
    print_mesh2node(gltf, buffer)
    print_mesh2material(gltf)


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)
        # generated_code(zip_file)


if __name__ == "__main__":
    main()
