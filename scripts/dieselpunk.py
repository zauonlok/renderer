"""Preprocess the Dieselpunk Duo model

The model is available for download from
    https://sketchfab.com/models/0163eca36af441ff9cca6f8c1bee1fdc

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile
from PIL import Image
import utils

SRC_FILENAME = "dieselpunk_duo.zip"
DST_DIRECTORY = "../assets/dieselpunk"

OBJ_FILENAMES = [
    "mech.obj",
    "yingham.obj",
    "ground.obj",
]

MECH_BASECOLOR_PATH = "textures/Mech_baseColor.png"
MECH_NORMAL_PATH = "textures/Mech_normal.png"
MECH_PACKED_PATH = "textures/Mech_metallicRoughness.png"
YINGHAM_BASECOLOR_PATH = "textures/Yingham_MAT_baseColor.png"
YINGHAM_NORMAL_PATH = "textures/Yingham_MAT_normal.png"
YINGHAM_PACKED_PATH = "textures/Yingham_MAT_metallicRoughness.png"
GROUND_BASECOLOR_PATH = "textures/Ground_baseColor.png"
GROUND_PACKED_PATH = "textures/Ground_metallicRoughness.png"


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    meshes = []
    for mesh in gltf["meshes"]:
        mesh = utils.load_gltf_mesh(gltf, buffer, mesh)
        meshes.append(mesh)

    for obj_filename, mesh in zip(OBJ_FILENAMES, meshes):
        obj_data, tan_data = utils.dump_mesh_data(mesh)

        obj_filepath = os.path.join(DST_DIRECTORY, obj_filename)
        with open(obj_filepath, "w") as f:
            f.write(obj_data)

        tan_filepath = obj_filepath[:-4] + ".tan"
        with open(tan_filepath, "w") as f:
            f.write(tan_data)


def load_image(zip_file, filename):
    with zip_file.open(filename) as f:
        image = Image.open(f)
        image = image.transpose(Image.FLIP_TOP_BOTTOM)
        return image


def save_image(image, filename, size=(512, 512)):
    image = image.resize(size, Image.LANCZOS)
    filepath = os.path.join(DST_DIRECTORY, filename)
    image.save(filepath, rle=True)


def process_mech_images(zip_file):
    basecolor_image = load_image(zip_file, MECH_BASECOLOR_PATH)
    save_image(basecolor_image, "mech_basecolor.tga")

    normal_image = load_image(zip_file, MECH_NORMAL_PATH)
    save_image(normal_image, "mech_normal.tga", size=(1024, 1024))

    packed_image = load_image(zip_file, MECH_PACKED_PATH)
    _, roughness_image, metallic_image = packed_image.split()
    save_image(metallic_image, "mech_metallic.tga")
    save_image(roughness_image, "mech_roughness.tga")


def process_yingham_images(zip_file):
    basecolor_image = load_image(zip_file, YINGHAM_BASECOLOR_PATH)
    save_image(basecolor_image, "yingham_basecolor.tga")

    normal_image = load_image(zip_file, YINGHAM_NORMAL_PATH)
    save_image(normal_image, "yingham_normal.tga", size=(1024, 1024))

    packed_image = load_image(zip_file, YINGHAM_PACKED_PATH)
    assert len(packed_image.split()) == 1
    roughness_image = packed_image.convert("L")
    save_image(roughness_image, "yingham_roughness.tga")


def process_ground_images(zip_file):
    basecolor_image = load_image(zip_file, GROUND_BASECOLOR_PATH)
    save_image(basecolor_image, "ground_basecolor.tga")

    packed_image = load_image(zip_file, GROUND_PACKED_PATH)
    assert len(packed_image.split()) == 1
    roughness_image = packed_image.convert("L")
    save_image(roughness_image, "ground_roughness.tga")


def process_images(zip_file):
    process_mech_images(zip_file)
    process_yingham_images(zip_file)
    process_ground_images(zip_file)


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
