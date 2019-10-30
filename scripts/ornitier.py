"""Preprocess the Vivi Ornitier model

The model is available for download from
    https://sketchfab.com/models/5edb4f4c8a4b47d2a735807f0f4646cc

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile

from PIL import Image

from .utils.gltf import dump_obj_data

SRC_FILENAME = "vivi_ornitier.zip"
DST_DIRECTORY = "../assets/ornitier"

OBJ_FILENAMES = [
    "feuga.obj",
    "legs.obj",
    "body.obj",
    "hat.obj",
    "coat.obj",
    "hands.obj",
    "base.obj",
]

IMG_FILENAMES = {
    "base": [
        "textures/Base_baseColor.png",
        "textures/Base_metallicRoughness.png",
        "textures/Base_normal.png",
        "textures/Base_emissive.png",
    ],
    "body": [
        "textures/Body.001_baseColor.png",
        None,
        None,
        "textures/Body.001_emissive.png",
    ],
    "coat": [
        "textures/Coat.001_baseColor.png",
        "textures/Coat.001_metallicRoughness.png",
        "textures/Coat.001_normal.png",
        None,
    ],
    "feuga": [
        "textures/Feuga_baseColor.png",
        None,
        None,
        None,
    ],
    "hands": [
        "textures/HandsRod.001_baseColor.png",
        "textures/HandsRod.001_metallicRoughness.png",
        "textures/HandsRod.001_normal.png",
        None,
    ],
    "hat": [
        "textures/Hat.001_baseColor.png",
        "textures/Hat.001_metallicRoughness.png",
        "textures/Hat.001_normal.png",
        None,
    ],
    "legs": [
        "textures/Legs.001_baseColor.png",
        "textures/Legs.001_metallicRoughness.png",
        "textures/Legs.001_normal.png",
        None,
    ],
}


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

    linear_emission = [1.000, 0.553, 0.072]
    srgb_emission = [int(pow(x, 1 / 2.2) * 255) for x in linear_emission]
    emission_image = Image.new("RGB", (512, 512), color=tuple(srgb_emission))
    save_image(emission_image, "feuga_emission.tga")


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
