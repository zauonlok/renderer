"""Preprocess the Assassin Gai model

The model is available for download from
    https://sketchfab.com/models/2e571c1e614a4244a1035c6c25e75c4d

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile

from PIL import Image

from utils.gltf import dump_obj_data, dump_skin_ani_data

SRC_FILENAME = "assassin_gai.zip"
DST_DIRECTORY = "../assets/assassin"

OBJ_FILENAMES = [
    "body.obj",
    "hair.obj",
    "weapon.obj",
    "face.obj",
]

IMG_FILENAMES = {
    "body": [
        "textures/Material_854_baseColor.png",
        "textures/Material_854_metallicRoughness.png",
        "textures/Material_854_normal.png",
    ],
    "face": [
        "textures/Material_850_baseColor.png",
        None,
        None,
    ],
    "hair": [
        "textures/Material_852_baseColor.png",
        "textures/Material_852_metallicRoughness.png",
        "textures/Material_852_normal.png",
    ],
    "weapon": [
        "textures/Material_853_baseColor.png",
        "textures/Material_853_metallicRoughness.png",
        "textures/Material_853_normal.png",
    ],
}


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    for mesh_index, obj_filename in enumerate(OBJ_FILENAMES):
        obj_data = dump_obj_data(
            gltf, buffer, mesh_index, with_skin=True, with_tangent=True
        )
        obj_filepath = os.path.join(DST_DIRECTORY, obj_filename)
        with open(obj_filepath, "w") as f:
            f.write(obj_data)

    ani_data = dump_skin_ani_data(gltf, buffer)
    ani_filepath = os.path.join(DST_DIRECTORY, "assassin.ani")
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

        if basecolor_path:
            basecolor_image = load_image(zip_file, basecolor_path)
            save_image(basecolor_image, "{}_basecolor.tga".format(name))

        if packed_path:
            packed_image = load_image(zip_file, packed_path)
            _, roughness_image, metalness_image = packed_image.split()
            save_image(roughness_image, "{}_roughness.tga".format(name))
            save_image(metalness_image, "{}_metalness.tga".format(name))

        if normal_path:
            normal_image = load_image(zip_file, normal_path)
            save_image(normal_image, "{}_normal.tga".format(name))


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
