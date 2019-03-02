"""Preprocess the Helmet Concept model

The model is available for download from
    https://sketchfab.com/models/83bdd9464ea4497bb687259afc4d8ae8

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile
from PIL import Image
from utils.gltf import dump_obj_data

SRC_FILENAME = "helmetconcept.zip"
DST_DIRECTORY = "../assets/helmet2"

OBJ_FILENAMES = {
    "helmet.obj",
    "glass.obj",
}


def process_mesh(zip_file):
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


def save_image(image, filename, size=(512, 512)):
    image = image.resize(size, Image.LANCZOS)
    filepath = os.path.join(DST_DIRECTORY, filename)
    image.save(filepath, rle=True)


def process_helmet_images(zip_file):
    basecolor_image = load_image(
        zip_file, "textures/LP_zlozone_lambert23SG1_baseColor.jpeg"
    )
    save_image(basecolor_image, "helmet_basecolor.tga")

    normal_image = load_image(
        zip_file, "textures/LP_zlozone_lambert23SG1_normal.png"
    )
    save_image(normal_image, "helmet_normal.tga", size=(1024, 1024))

    emissive_image = load_image(
        zip_file, "textures/LP_zlozone_lambert23SG1_emissive.jpeg"
    )
    save_image(emissive_image, "helmet_emissive.tga")

    packed_image = load_image(
        zip_file, "textures/LP_zlozone_lambert23SG1_metallicRoughness.png"
    )
    occlusion_image, roughness_image, metallic_image = packed_image.split()
    save_image(metallic_image, "helmet_metallic.tga")
    save_image(roughness_image, "helmet_roughness.tga")
    save_image(occlusion_image, "helmet_occlusion.tga")


def process_glass_images(zip_file):
    packed_image = load_image(
        zip_file, "textures/LP_zlozone_lambert25SG1_metallicRoughness.png"
    )
    assert packed_image.mode == "P"
    _, roughness_image, _, _ = packed_image.convert("RGBA").split()
    save_image(roughness_image, "glass_roughness.tga")


def process_images(zip_file):
    process_helmet_images(zip_file)
    process_glass_images(zip_file)


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_mesh(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
