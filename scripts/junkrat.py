"""Preprocess the Junkrat model

The model is available for download from
    https://sketchfab.com/models/7deb2dd552df4bf4bb65005018176647

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile
from PIL import Image
from utils.gltf import dump_ani_data, dump_obj_data

SRC_FILENAME = "junkrat.zip"
DST_DIRECTORY = "../assets/junkrat"

IMG_FILENAMES = {
    "textures/Material_22_baseColor.png": "upper.tga",
    "textures/Material_19_baseColor.png": "lower.tga",
    "textures/Material_34_baseColor.png": "head.tga",
    "textures/Material_20_baseColor.png": "back.tga",
}


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    for mesh_index in range(len(gltf["meshes"])):
        obj_data = dump_obj_data(gltf, buffer, mesh_index, with_skin=True)
        obj_filename = "junkrat{}.obj".format(mesh_index)
        obj_filepath = os.path.join(DST_DIRECTORY, obj_filename)
        with open(obj_filepath, "w") as f:
            f.write(obj_data)

    ani_data = dump_ani_data(gltf, buffer)
    ani_filepath = os.path.join(DST_DIRECTORY, "junkrat.ani")
    with open(ani_filepath, "w") as f:
            f.write(ani_data)


def process_images(zip_file):
    for old_filename, tga_filename in IMG_FILENAMES.items():
        with zip_file.open(old_filename) as f:
            image = Image.open(f)
            image = image.transpose(Image.FLIP_TOP_BOTTOM)
            image = image.resize((512, 512), Image.LANCZOS)
            filepath = os.path.join(DST_DIRECTORY, tga_filename)
            image.save(filepath, rle=True)


def print_mesh2material(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    meshes = gltf["meshes"]

    mesh2material = []
    for mesh in meshes:
        assert len(mesh["primitives"]) == 1
        primitive = mesh["primitives"][0]
        mesh2material.append(primitive["material"])

    num_meshes = len(mesh2material)
    chunk_size = num_meshes / 3 + 1
    print("    int mesh2material[{}] = {{".format(num_meshes))
    for i in range(0, num_meshes, chunk_size):
        indices = [str(j) for j in mesh2material[i:(i + chunk_size)]]
        print("        {}".format(", ".join(indices) + ","))
    print("    };")


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)
        # print_mesh2material(zip_file)


if __name__ == "__main__":
    main()
