"""Preprocess the Robot model

The model is available for download from
    https://sketchfab.com/3d-models/uaeYu2fwakD1e1bWp5Cxu3XAqrt

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile

from PIL import Image

from utils.gltf import dump_obj_data

SRC_FILENAME = "robo_obj_pose4.zip"
DST_DIRECTORY = "../assets/robot"


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    for mesh_index in range(len(gltf["meshes"])):
        obj_data = dump_obj_data(gltf, buffer, mesh_index)
        filename = "robot{}.obj".format(mesh_index)
        filepath = os.path.join(DST_DIRECTORY, filename)
        with open(filepath, "w") as f:
            f.write(obj_data)


def load_image(zip_file, filename):
    with zip_file.open(filename) as f:
        image = Image.open(f)
        image = image.transpose(Image.FLIP_TOP_BOTTOM)
        return image


def save_image(image, filename, size=512):
    if max(image.size) > size:
        image = image.resize((size, size), Image.LANCZOS)
    filepath = os.path.join(DST_DIRECTORY, filename)
    image.save(filepath, rle=True)


def process_images(zip_file):
    basecolor_path = "textures/material_0_baseColor.jpeg"
    emission_path = "textures/material_0_emissive.jpeg"
    packed_path = "textures/material_0_metallicRoughness.png"

    basecolor_image = load_image(zip_file, basecolor_path)
    save_image(basecolor_image, "robot_basecolor.tga")

    emission_image = load_image(zip_file, emission_path)
    save_image(emission_image, "robot_emission.tga")

    packed_image = load_image(zip_file, packed_path)
    _, roughness_image, _, _ = packed_image.convert("RGBA").split()
    save_image(roughness_image, "robot_roughness.tga")


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
