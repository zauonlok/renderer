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

OBJ_FILENAMES = [
    "helmet.obj",
    "glass.obj",
]

IMG_FILENAMES = {
    "helmet": [
        "textures/LP_zlozone_lambert23SG1_baseColor.jpeg",
        "textures/LP_zlozone_lambert23SG1_emissive.jpeg",
        "textures/LP_zlozone_lambert23SG1_metallicRoughness.png",
    ],
    "glass": [
        "textures/LP_zlozone_lambert25SG1_baseColor.png",
        "textures/LP_zlozone_lambert25SG1_emissive.jpeg",
        "textures/LP_zlozone_lambert25SG1_metallicRoughness.png",
    ],
}

DEL_FILENAMES = [
    "glass_basecolor.tga",
    "glass_emission.tga",
    "glass_metalness.tga",
    "glass_occlusion.tga",
]


def process_mesh(zip_file):
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
    for name, paths in IMG_FILENAMES.items():
        basecolor_path, emission_path, packed_path = paths

        basecolor_image = load_image(zip_file, basecolor_path)
        if basecolor_image.mode == "P":
            basecolor_image = basecolor_image.convert("RGBA")
        save_image(basecolor_image, "{}_basecolor.tga".format(name))

        emission_image = load_image(zip_file, emission_path)
        save_image(emission_image, "{}_emission.tga".format(name))

        packed_image = load_image(zip_file, packed_path)
        if packed_image.mode == "P":
            packed_bands = packed_image.convert("RGBA").split()[:3]
        else:
            packed_bands = packed_image.split()
        occlusion_image, roughness_image, metalness_image = packed_bands
        save_image(occlusion_image, "{}_occlusion.tga".format(name))
        save_image(roughness_image, "{}_roughness.tga".format(name))
        save_image(metalness_image, "{}_metalness.tga".format(name))

    for del_filename in DEL_FILENAMES:
        del_filepath = os.path.join(DST_DIRECTORY, del_filename)
        os.remove(del_filepath)


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_mesh(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
