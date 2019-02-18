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
import utils

SRC_FILENAME = "helmetconcept.zip"
DST_DIRECTORY = "../assets/helmet2"
OBJ_FILENAME = "helmet.obj"

BASECOLOR_PATH = "textures/LP_zlozone_lambert23SG1_baseColor.jpeg"
NORMAL_PATH = "textures/LP_zlozone_lambert23SG1_normal.png"
EMISSIVE_PATH = "textures/LP_zlozone_lambert23SG1_emissive.jpeg"
PACKED_PATH = "textures/LP_zlozone_lambert23SG1_metallicRoughness.png"


def process_mesh(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    mesh = utils.load_gltf_mesh(gltf, buffer, gltf["meshes"][0])
    obj_data, tan_data = utils.dump_mesh_data(mesh)

    obj_filepath = os.path.join(DST_DIRECTORY, OBJ_FILENAME)
    with open(obj_filepath, "w") as f:
        f.write(obj_data)

    tan_filepath = obj_filepath[:-4] + ".tan"
    with open(tan_filepath, "w") as f:
        f.write(tan_data)


def load_image(zip_file, filename):
    with zip_file.open(filename) as f:
        image = Image.open(f)
        bands = image.split()
        image = Image.merge("RGB", bands[:3])
        image = image.transpose(Image.FLIP_TOP_BOTTOM)
        return image


def save_image(image, filename, resize_first):
    if resize_first:
        image = image.resize((512, 512), Image.LANCZOS)
    filepath = os.path.join(DST_DIRECTORY, filename)
    image.save(filepath, rle=True)


def process_images(zip_file):
    basecolor_image = load_image(zip_file, BASECOLOR_PATH)
    save_image(basecolor_image, "basecolor.tga", True)

    normal_image = load_image(zip_file, NORMAL_PATH)
    normal_image = normal_image.resize((1024, 1024), Image.LANCZOS)
    save_image(normal_image, "normal.tga", False)

    emissive_image = load_image(zip_file, EMISSIVE_PATH)
    save_image(emissive_image, "emissive.tga", True)

    packed_image = load_image(zip_file, PACKED_PATH)
    occlusion_image, roughness_image, metallic_image = packed_image.split()
    save_image(metallic_image, "metallic.tga", True)
    save_image(roughness_image, "roughness.tga", True)
    save_image(occlusion_image, "occlusion.tga", True)


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_mesh(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
