"""Preprocess the Supermarine Spitfire model

The model is available for download from
    https://sketchfab.com/models/8349f26e1e88455da75dd7352b02b794

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile
from PIL import Image
from utils.gltf import dump_obj_data

SRC_FILENAME = "supermarine_spitfire.zip"
DST_DIRECTORY = "../assets/spitfire"

OBJ_FILENAMES = [
    "body.obj",
    "windows.obj",
]

IMG_FILENAMES = [
    "textures/Material_85_baseColor.png",
    "textures/Material_85_metallicRoughness.png",
]


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    for mesh_index, filename in enumerate(OBJ_FILENAMES):
        obj_data = dump_obj_data(gltf, buffer, mesh_index)
        filepath = os.path.join(DST_DIRECTORY, filename)
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
    basecolor_path, packed_path = IMG_FILENAMES

    basecolor_image = load_image(zip_file, basecolor_path)
    save_image(basecolor_image, "spitfire_basecolor.tga")

    packed_image = load_image(zip_file, packed_path)
    occlusion_image, roughness_image, metalness_image = packed_image.split()
    save_image(occlusion_image, "spitfire_occlusion.tga")
    save_image(roughness_image, "spitfire_roughness.tga")
    save_image(metalness_image, "spitfire_metalness.tga")


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
