"""Preprocess the Vivi Ornitier model

The model is available for download from
    https://sketchfab.com/models/5edb4f4c8a4b47d2a735807f0f4646cc

The Python Imaging Library and NumPy are required
    pip install pillow numpy
"""

from __future__ import print_function

import json
import os
import zipfile
from PIL import Image
import numpy
import utils

SRC_FILENAME = "vivi_ornitier.zip"
DST_DIRECTORY = "../assets/ornitier"

OBJ_FILENAMES = [
    None,
    "legs.obj",
    "body.obj",
    "hat.obj",
    "coat.obj",
    "hands.obj",
    "base.obj",
]

EMISSIVE_FILENAMES = {
    "textures/Base_emissive.png": "base_emission.tga",
    "textures/Body.001_emissive.png": "body_emission.tga",
}

METALLIC_FILENAMES = {
    "base": [
        "textures/Base_baseColor.png",
        "textures/Base_metallicRoughness.png",
    ],
    "body": [
        "textures/Body.001_baseColor.png",
        "textures/Body.001_metallicRoughness.png",
    ],
    "coat": [
        "textures/Coat.001_baseColor.png",
        "textures/Coat.001_metallicRoughness.png",
    ],
    "hands": [
        "textures/HandsRod.001_baseColor.png",
        "textures/HandsRod.001_metallicRoughness.png",
    ],
    "hat": [
        "textures/Hat.001_baseColor.png",
        "textures/Hat.001_metallicRoughness.png",
    ],
    "legs": [
        "textures/Legs.001_baseColor.png",
        "textures/Legs.001_metallicRoughness.png",
    ],
}


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    meshes = []
    for mesh in gltf["meshes"]:
        mesh = utils.load_gltf_mesh(gltf, buffer, mesh)
        meshes.append(mesh)

    for filename, mesh in zip(OBJ_FILENAMES, meshes):
        if filename:
            obj_data, _ = utils.dump_mesh_data(mesh)
            filepath = os.path.join(DST_DIRECTORY, filename)
            with open(filepath, "w") as f:
                f.write(obj_data)


def load_rgb_image(zip_file, filename):
    with zip_file.open(filename) as f:
        image = Image.open(f)
        bands = image.split()
        image = Image.merge("RGB", bands[:3])
        image = image.transpose(Image.FLIP_TOP_BOTTOM)
        image = image.resize((512, 512), Image.LANCZOS)
        return image


def process_emissive_images(zip_file):
    for old_filename, tga_filename in EMISSIVE_FILENAMES.items():
        image = load_rgb_image(zip_file, old_filename)
        filepath = os.path.join(DST_DIRECTORY, tga_filename)
        image.save(filepath, rle=True)


def process_metallic_images(zip_file):
    for name, (basecolor_path, metallic_path) in METALLIC_FILENAMES.items():
        basecolor_image = load_rgb_image(zip_file, basecolor_path)
        metallic_image = load_rgb_image(zip_file, metallic_path)
        _, _, metallic_image = metallic_image.split()

        basecolor_array = numpy.array(basecolor_image) / 255.0
        metallic_array = (numpy.array(metallic_image) / 255.0)[:, :, None]

        diffuse_array = basecolor_array * (1 - metallic_array)
        specular_array = basecolor_array * metallic_array

        diffuse_image = Image.fromarray(numpy.uint8(diffuse_array * 255.0))
        specular_image = Image.fromarray(numpy.uint8(specular_array * 255.0))

        diffuse_filename = "{}_diffuse.tga".format(name)
        specular_filename = "{}_specular.tga".format(name)
        diffuse_filepath = os.path.join(DST_DIRECTORY, diffuse_filename)
        specular_filepath = os.path.join(DST_DIRECTORY, specular_filename)

        diffuse_image.save(diffuse_filepath, rle=True)
        specular_image.save(specular_filepath, rle=True)


def process_images(zip_file):
    process_emissive_images(zip_file)
    process_metallic_images(zip_file)


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
