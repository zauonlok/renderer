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

from .utils.gltf import dump_obj_data

SRC_FILENAME = "pony_cartoon.zip"
DST_DIRECTORY = "../assets/ponycar"

OBJ_FILENAMES = [
    "body.obj",
    None,
    "interior.obj",
    "windows.obj",
]

IMG_FILENAMES = {
    "body": [
        "textures/Body_SG1_baseColor.jpeg",
        "textures/Body_SG1_metallicRoughness.png",
        "textures/Body_SG1_normal.jpg",
        "textures/Body_SG1_emissive.jpeg",
    ],
    "interior": [
        "textures/Interior_SG_baseColor.jpg",
        "textures/Interior_SG_metallicRoughness.png",
        "textures/Interior_SG_normal.jpg",
        None,
    ],
}


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


def save_image(image, filename, size=512):
    if max(image.size) > size:
        image = image.resize((size, size), Image.LANCZOS)
    filepath = os.path.join(DST_DIRECTORY, filename)
    image.save(filepath, rle=True)


def process_images(zip_file):
    for name, paths in IMG_FILENAMES.items():
        basecolor_path, packed_path, normal_path, emission_path = paths

        if basecolor_path:
            basecolor_image = load_image(zip_file, basecolor_path)
            save_image(basecolor_image, "{}_basecolor.tga".format(name))

        if packed_path:
            packed_image = load_image(zip_file, packed_path)
            _, roughness_image, metalness_image = packed_image.split()
            save_image(roughness_image, "{}_roughness.tga".format(name))
            save_image(metalness_image, "{}_metalness.tga".format(name))

        if normal_path:
            normal_image = load_image(zip_file, normal_path)
            save_image(normal_image, "{}_normal.tga".format(name))

        if emission_path:
            emission_image = load_image(zip_file, emission_path)
            save_image(emission_image, "{}_emission.tga".format(name))


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
