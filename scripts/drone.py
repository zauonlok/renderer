"""Preprocess the Mech Drone model

The model is available for download from
    https://sketchfab.com/models/8d06874aac5246c59edb4adbe3606e0e

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile
from PIL import Image
from utils.gltf import dump_obj_data

SRC_FILENAME = "mech_drone.zip"
DST_DIRECTORY = "../assets/drone"


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    obj_data = dump_obj_data(gltf, buffer, 0)
    filepath = os.path.join(DST_DIRECTORY, "drone.obj")
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


def process_images(zip_file):
    diffuse_image = load_image(zip_file, "textures/Robot_diffuse.jpeg")
    save_image(diffuse_image, "diffuse.tga")

    occlusion_image = load_image(zip_file, "textures/Robot_occlusion.jpeg")
    occlusion_image, _, _ = occlusion_image.split()
    save_image(occlusion_image, "occlusion.tga")

    emissive_image = load_image(zip_file, "textures/Robot_emissive.jpeg")
    save_image(emissive_image, "emissive.tga")

    packed_image = load_image(zip_file, "textures/Robot_specularGlossiness.png")
    packed_bands = packed_image.split()

    specular_image = Image.merge("RGB", packed_bands[:3])
    glossiness_image = packed_bands[3]
    save_image(specular_image, "specular.tga")
    save_image(glossiness_image, "glossiness.tga")


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
