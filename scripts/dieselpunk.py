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

from .utils.gltf import dump_obj_data

SRC_FILENAME = "dieselpunk_duo.zip"
DST_DIRECTORY = "../assets/dieselpunk"

OBJ_FILENAMES = [
    "mech.obj",
    "yingham.obj",
    "ground.obj",
]

IMG_FILENAMES = {
    "ground": [
        "textures/Ground_baseColor.png",
        "textures/Ground_metallicRoughness.png",
        None,
    ],
    "mech": [
        "textures/Mech_baseColor.png",
        "textures/Mech_metallicRoughness.png",
        "textures/Mech_normal.png",
    ],
    "yingham": [
        "textures/Yingham_MAT_baseColor.png",
        "textures/Yingham_MAT_metallicRoughness.png",
        "textures/Yingham_MAT_normal.png",
    ],
}

DEL_FILENAMES = [
    "ground_metalness.tga",
    "yingham_metalness.tga",
]


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    for mesh_index, filename in enumerate(OBJ_FILENAMES):
        obj_data = dump_obj_data(gltf, buffer, mesh_index, with_tangent=True)
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
        basecolor_path, packed_path, normal_path = paths

        if basecolor_path:
            basecolor_image = load_image(zip_file, basecolor_path)
            save_image(basecolor_image, "{}_basecolor.tga".format(name))

        if packed_path:
            packed_image = load_image(zip_file, packed_path)
            if packed_image.mode == "P":
                packed_bands = packed_image.convert("RGBA").split()[:3]
            else:
                packed_bands = packed_image.split()
            _, roughness_image, metalness_image = packed_bands
            save_image(roughness_image, "{}_roughness.tga".format(name))
            save_image(metalness_image, "{}_metalness.tga".format(name))

        if normal_path:
            normal_image = load_image(zip_file, normal_path)
            save_image(normal_image, "{}_normal.tga".format(name))

    for del_filename in DEL_FILENAMES:
        del_filepath = os.path.join(DST_DIRECTORY, del_filename)
        os.remove(del_filepath)


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
