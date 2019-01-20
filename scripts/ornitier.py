"""Preprocess the Vivi Ornitier model

The model is available for download from
    https://sketchfab.com/models/5edb4f4c8a4b47d2a735807f0f4646cc

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile
from PIL import Image
import utils

SRC_FILENAME = "vivi_ornitier.zip"
DST_DIRECTORY = "../assets/ornitier"

OBJ_FILENAMES = [
    None,
    "legs.obj",
    "body.obj",
    "hat.obj",
    "coat.obj",
    "hands.obj",
    "base.obj",
]

IMG_FILENAMES = {
    # base textures
    "textures/Base_baseColor.png": "base_diffuse.tga",
    "textures/Base_emissive.png": "base_emission.tga",
    "textures/Base_metallicRoughness.png": "base_specular.tga",
    # body textures
    "textures/Body.001_baseColor.png": "body_diffuse.tga",
    "textures/Body.001_emissive.png": "body_emission.tga",
    "textures/Body.001_metallicRoughness.png": "body_specular.tga",
    # coat textures
    "textures/Coat.001_baseColor.png": "coat_diffuse.tga",
    "textures/Coat.001_metallicRoughness.png": "coat_specular.tga",
    # hands textures
    "textures/HandsRod.001_baseColor.png": "hands_diffuse.tga",
    "textures/HandsRod.001_metallicRoughness.png": "hands_specular.tga",
    # hat textures
    "textures/Hat.001_baseColor.png": "hat_diffuse.tga",
    "textures/Hat.001_metallicRoughness.png": "hat_specular.tga",
    # legs textures
    "textures/Legs.001_baseColor.png": "legs_diffuse.tga",
    "textures/Legs.001_metallicRoughness.png": "legs_specular.tga",
}


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    meshes = []
    for mesh in gltf["meshes"]:
        mesh = utils.load_gltf_mesh(gltf, buffer, mesh)
        meshes.append(mesh)

    for filename, mesh in zip(OBJ_FILENAMES, meshes):
        if filename:
            content = utils.dump_mesh_data(mesh)
            filepath = os.path.join(DST_DIRECTORY, filename)
            with open(filepath, "w") as f:
                f.write(content)


def process_images(zip_file):
    for old_filename, tga_filename in IMG_FILENAMES.items():
        with zip_file.open(old_filename) as f:
            image = Image.open(f)
            if "metallicRoughness" in old_filename:
                _, _, blue = image.split()
                image = blue
            else:
                bands = image.split()
                image = Image.merge("RGB", bands[:3])
            image = image.transpose(Image.FLIP_TOP_BOTTOM)
            image = image.resize((512, 512), Image.LANCZOS)
            filepath = os.path.join(DST_DIRECTORY, tga_filename)
            image.save(filepath, rle=True)


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
