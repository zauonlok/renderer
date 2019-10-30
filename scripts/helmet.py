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

from .utils.gltf import dump_obj_data

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


def save_image(image, filename, size=1024):
    if max(image.size) > size:
        image = image.resize((size, size), Image.LANCZOS)
    filepath = os.path.join(DST_DIRECTORY, filename)
    image.save(filepath, rle=True)


def process_images(zip_file):
    basecolor_path = MODEL_DIR + "Default_albedo.jpg"
    packed_path = MODEL_DIR + "Default_metalRoughness.jpg"
    occlusion_path = MODEL_DIR + "Default_AO.jpg"
    normal_path = MODEL_DIR + "Default_normal.jpg"
    emission_path = MODEL_DIR + "Default_emissive.jpg"

    basecolor_image = load_image(zip_file, basecolor_path)
    save_image(basecolor_image, "helmet_basecolor.tga")

    packed_image = load_image(zip_file, packed_path)
    _, roughness_image, metalness_image = packed_image.split()
    save_image(roughness_image, "helmet_roughness.tga")
    save_image(metalness_image, "helmet_metalness.tga")

    occlusion_image = load_image(zip_file, occlusion_path)
    occlusion_image = occlusion_image.split()[0]
    save_image(occlusion_image, "helmet_occlusion.tga")

    normal_image = load_image(zip_file, normal_path)
    save_image(normal_image, "helmet_normal.tga")

    emission_image = load_image(zip_file, emission_path)
    save_image(emission_image, "helmet_emission.tga")


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        # process_mesh(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
