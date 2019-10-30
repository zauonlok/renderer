"""Preprocess the Junkrat model

The model is available for download from
    https://sketchfab.com/models/7deb2dd552df4bf4bb65005018176647

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile

from PIL import Image

from .utils.gltf import dump_obj_data, dump_skin_ani_data

SRC_FILENAME = "junkrat.zip"
DST_DIRECTORY = "../assets/junkrat"

IMG_FILENAMES = {
    "back": [
        "textures/Material_20_baseColor.png",
        "textures/Material_20_metallicRoughness.png",
        "textures/Material_20_normal.png",
    ],
    "head": [
        "textures/Material_34_baseColor.png",
        "textures/Material_34_metallicRoughness.png",
        "textures/Material_34_normal.png",
    ],
    "lower": [
        "textures/Material_19_baseColor.png",
        "textures/Material_19_metallicRoughness.png",
        "textures/Material_19_normal.png",
    ],
    "upper": [
        "textures/Material_22_baseColor.png",
        "textures/Material_22_metallicRoughness.png",
        "textures/Material_22_normal.png",
    ],
}

DEL_FILENAMES = [
    "back_roughness.tga",
    "head_metalness.tga",
    "lower_roughness.tga",
    "upper_roughness.tga",
]


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    for mesh_index in range(len(gltf["meshes"])):
        obj_data = dump_obj_data(
            gltf, buffer, mesh_index, with_skin=True, with_tangent=True
        )
        obj_filename = "junkrat{}.obj".format(mesh_index)
        obj_filepath = os.path.join(DST_DIRECTORY, obj_filename)
        with open(obj_filepath, "w") as f:
            f.write(obj_data)

    ani_data = dump_skin_ani_data(gltf, buffer)
    ani_filepath = os.path.join(DST_DIRECTORY, "junkrat.ani")
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
    for name, paths in IMG_FILENAMES.items():
        basecolor_path, packed_path, normal_path = paths

        basecolor_image = load_image(zip_file, basecolor_path)
        save_image(basecolor_image, "{}_basecolor.tga".format(name))

        packed_image = load_image(zip_file, packed_path)
        _, roughness_image, metalness_image = packed_image.split()
        save_image(roughness_image, "{}_roughness.tga".format(name))
        save_image(metalness_image, "{}_metalness.tga".format(name))

        normal_image = load_image(zip_file, normal_path)
        save_image(normal_image, "{}_normal.tga".format(name))

    for del_filename in DEL_FILENAMES:
        del_filepath = os.path.join(DST_DIRECTORY, del_filename)
        os.remove(del_filepath)


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
