"""Preprocess the Wood Toy Horse model

The model is available for download from
    https://sketchfab.com/3d-models/e114132dd22b43b5be17ae543697c9e8

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile

from PIL import Image

from utils.gltf import dump_obj_data, dump_skin_ani_data

SRC_FILENAME = "woodtoy_horse.zip"
DST_DIRECTORY = "../assets/horse"


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    for mesh_index in range(len(gltf["meshes"])):
        obj_data = dump_obj_data(gltf, buffer, mesh_index, with_tangent=True)
        obj_filename = "horse{}.obj".format(mesh_index)
        obj_filepath = os.path.join(DST_DIRECTORY, obj_filename)
        with open(obj_filepath, "w") as f:
            f.write(obj_data)

    ani_data = dump_skin_ani_data(gltf, buffer, animation_index=3)
    ani_filepath = os.path.join(DST_DIRECTORY, "horse.ani")
    with open(ani_filepath, "w") as f:
        f.write(ani_data)


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
    basecolor_path = "textures/Mat_CAballoMadera_baseColor.png"
    normal_path = "textures/Mat_CAballoMadera_normal.png"
    packed_path = "textures/Mat_CAballoMadera_metallicRoughness.png"

    basecolor_image = load_image(zip_file, basecolor_path)
    save_image(basecolor_image, "horse_basecolor.tga")

    normal_image = load_image(zip_file, normal_path)
    save_image(normal_image, "horse_normal.tga")

    packed_image = load_image(zip_file, packed_path)
    _, roughness_image, metalness_image = packed_image.split()
    save_image(roughness_image, "horse_roughness.tga")
    save_image(metalness_image, "horse_metalness.tga")


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
