"""Preprocess the Damaged Helmet model

The model is available for download from
    https://github.com/KhronosGroup/glTF-Sample-Models/archive/master.zip

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile
from PIL import Image
from utils.gltf import dump_obj_data

SRC_FILENAME = "glTF-Sample-Models-master.zip"
DST_DIRECTORY = "../assets/helmet"
MODEL_DIR = "glTF-Sample-Models-master/2.0/DamagedHelmet/glTF/"


def process_mesh(zip_file):
    gltf = json.loads(zip_file.read(MODEL_DIR + "DamagedHelmet.gltf"))
    buffer = zip_file.read(MODEL_DIR + "DamagedHelmet.bin")

    obj_data = dump_obj_data(gltf, buffer, 0)
    filepath = os.path.join(DST_DIRECTORY, "helmet.obj")
    with open(filepath, "w") as f:
        f.write(obj_data)


def load_image(zip_file, filename):
    with zip_file.open(filename) as f:
        image = Image.open(f)
        image = image.transpose(Image.FLIP_TOP_BOTTOM)
        return image


def save_image(image, filename):
    image = image.resize((512, 512), Image.LANCZOS)
    filepath = os.path.join(DST_DIRECTORY, filename)
    image.save(filepath, rle=True)


def process_images(zip_file):
    basecolor_image = load_image(zip_file, MODEL_DIR + "Default_albedo.jpg")
    save_image(basecolor_image, "basecolor.tga")

    packed_image = load_image(
        zip_file, MODEL_DIR + "Default_metalRoughness.jpg"
    )
    _, roughness_image, metallic_image = packed_image.split()
    save_image(metallic_image, "metallic.tga")
    save_image(roughness_image, "roughness.tga")

    occlusion_image = load_image(zip_file, MODEL_DIR + "Default_AO.jpg")
    occlusion_image, _, _ = occlusion_image.split()
    save_image(occlusion_image, "occlusion.tga")

    emissive_image = load_image(zip_file, MODEL_DIR + "Default_emissive.jpg")
    save_image(emissive_image, "emissive.tga")


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_mesh(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
