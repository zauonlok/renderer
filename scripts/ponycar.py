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
from utils.gltf import dump_obj_data

SRC_FILENAME = "pony_cartoon.zip"
DST_DIRECTORY = "../assets/ponycar"

OBJ_FILENAMES = [
    "body.obj",
    None,
    "interior.obj",
    "windows.obj",
]


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    for mesh_index, filename in enumerate(OBJ_FILENAMES):
        if filename:
            obj_data = dump_obj_data(
                gltf, buffer, mesh_index, with_tangent=True
            )
            filepath = os.path.join(DST_DIRECTORY, filename)
            with open(filepath, "w") as f:
                f.write(obj_data)


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
    basecolor_image = load_image(zip_file, "textures/Body_SG1_baseColor.jpeg")
    save_image(basecolor_image, "body_basecolor.tga")

    emissive_image = load_image(zip_file, "textures/Body_SG1_emissive.jpeg")
    save_image(emissive_image, "body_emissive.tga")

    normal_image = load_image(zip_file, "textures/Body_SG1_normal.jpg")
    save_image(normal_image, "body_normal.tga", size=(1024, 1024))

    packed_image = load_image(
        zip_file, "textures/Body_SG1_metallicRoughness.png"
    )
    _, roughness_image, metallic_image = packed_image.split()
    save_image(metallic_image, "body_metallic.tga")
    save_image(roughness_image, "body_roughness.tga")


def process_interior_images(zip_file):
    basecolor_image = load_image(zip_file, "textures/Interior_SG_baseColor.jpg")
    save_image(basecolor_image, "interior_basecolor.tga")

    normal_image = load_image(zip_file, "textures/Interior_SG_normal.jpg")
    save_image(normal_image, "interior_normal.tga", size=(1024, 1024))

    packed_image = load_image(
        zip_file, "textures/Interior_SG_metallicRoughness.png"
    )
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
