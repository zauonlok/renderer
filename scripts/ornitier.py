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
    "feuga.obj",
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
        obj_data, _ = utils.dump_mesh_data(mesh)
        filepath = os.path.join(DST_DIRECTORY, filename)
        with open(filepath, "w") as f:
            f.write(obj_data)


def load_image(zip_file, filename):
    with zip_file.open(filename) as f:
        image = Image.open(f)
        image = image.transpose(Image.FLIP_TOP_BOTTOM)
        image = image.resize((512, 512), Image.LANCZOS)
        return image


def save_image(image, filename):
    filepath = os.path.join(DST_DIRECTORY, filename)
    image.save(filepath, rle=True)


def process_feuga_images(zip_file):
    basecolor_image = load_image(zip_file, "textures/Feuga_baseColor.png")
    _, _, _, alpha = basecolor_image.split()
    albedo = Image.new("L", (512, 512), color=160)
    diffuse_image = Image.merge("RGBA", [albedo, albedo, albedo, alpha])
    save_image(diffuse_image, "feuga_diffuse.tga")

    linear_emission = [1.000, 0.553, 0.072]
    srgb_emission = [int(pow(x, 1 / 2.2) * 255) for x in linear_emission]
    emission_image = Image.new("RGB", (512, 512), color=tuple(srgb_emission))
    save_image(emission_image, "feuga_emission.tga")


def process_emissive_images(zip_file):
    for old_filename, tga_filename in EMISSIVE_FILENAMES.items():
        image = load_image(zip_file, old_filename)
        save_image(image, tga_filename)


def process_metallic_images(zip_file):
    for name, (basecolor_path, metallic_path) in METALLIC_FILENAMES.items():
        basecolor_image = load_image(zip_file, basecolor_path)
        metallic_image = load_image(zip_file, metallic_path)
        _, _, metallic_image = metallic_image.split()

        basecolor_array = numpy.array(basecolor_image) / 255.0
        metallic_array = (numpy.array(metallic_image) / 255.0)[:, :, None]

        diffuse_array = basecolor_array * (1 - metallic_array)
        specular_array = basecolor_array * metallic_array

        diffuse_image = Image.fromarray(numpy.uint8(diffuse_array * 255.0))
        specular_image = Image.fromarray(numpy.uint8(specular_array * 255.0))

        diffuse_filename = "{}_diffuse.tga".format(name)
        specular_filename = "{}_specular.tga".format(name)
        save_image(diffuse_image, diffuse_filename)
        save_image(specular_image, specular_filename)


def process_images(zip_file):
    process_feuga_images(zip_file)
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
