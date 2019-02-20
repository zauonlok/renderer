"""Preprocess the Pony Cartoon model

The model is available for download from
    https://sketchfab.com/models/885d9f60b3a9429bb4077cfac5653cf9

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile
from PIL import Image
import utils

SRC_FILENAME = "pony_cartoon.zip"
DST_DIRECTORY = "../assets/ponycar"

OBJ_FILENAMES = [
    "body.obj",
    None,
    "interior.obj",
    "windows.obj",
]

BODY_BASECOLOR_PATH = "textures/Body_SG1_baseColor.jpeg"
BODY_EMISSIVE_PATH = "textures/Body_SG1_emissive.jpeg"
BODY_NORMAL_PATH = "textures/Body_SG1_normal.jpg"
BODY_PACKED_PATH = "textures/Body_SG1_metallicRoughness.png"
INTERIOR_BASECOLOR_PATH = "textures/Interior_SG_baseColor.jpg"
INTERIOR_NORMAL_PATH = "textures/Interior_SG_normal.jpg"
INTERIOR_PACKED_PATH = "textures/Interior_SG_metallicRoughness.png"


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    meshes = []
    for mesh in gltf["meshes"]:
        mesh = utils.load_gltf_mesh(gltf, buffer, mesh)
        meshes.append(mesh)

    for obj_filename, mesh in zip(OBJ_FILENAMES, meshes):
        if obj_filename:
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


def process_body_images(zip_file):
    basecolor_image = load_image(zip_file, BODY_BASECOLOR_PATH)
    save_image(basecolor_image, "body_basecolor.tga")

    emissive_image = load_image(zip_file, BODY_EMISSIVE_PATH)
    save_image(emissive_image, "body_emissive.tga")

    normal_image = load_image(zip_file, BODY_NORMAL_PATH)
    save_image(normal_image, "body_normal.tga", size=(1024, 1024))

    packed_image = load_image(zip_file, BODY_PACKED_PATH)
    _, roughness_image, metallic_image = packed_image.split()
    save_image(metallic_image, "body_metallic.tga")
    save_image(roughness_image, "body_roughness.tga")


def process_interior_images(zip_file):
    basecolor_image = load_image(zip_file, INTERIOR_BASECOLOR_PATH)
    save_image(basecolor_image, "interior_basecolor.tga")

    normal_image = load_image(zip_file, INTERIOR_NORMAL_PATH)
    save_image(normal_image, "interior_normal.tga", size=(1024, 1024))

    packed_image = load_image(zip_file, INTERIOR_PACKED_PATH)
    _, roughness_image, metallic_image = packed_image.split()
    save_image(metallic_image, "interior_metallic.tga")
    save_image(roughness_image, "interior_roughness.tga")


def process_images(zip_file):
    process_body_images(zip_file)
    process_interior_images(zip_file)


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
