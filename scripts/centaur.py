"""Preprocess the Centaur model

The model is available for download from
    https://sketchfab.com/models/0d3f1b4a51144b7fbc4e2ff64d858413

The Python Imaging Library and NumPy are required
    pip install pillow numpy
"""

from __future__ import print_function

import json
import os
import zipfile
from PIL import Image
import numpy
from utils.gltf import dump_obj_data

SRC_FILENAME = "centaur.zip"
DST_DIRECTORY = "../assets/centaur"

OBJ_FILENAMES = [
    "body.obj",
    None,
    "flame.obj",
    "gas.obj",
]

IMG_FILENAMES = {
    # body textures
    "textures/body_diffuse.jpeg": "body_diffuse.tga",
    "textures/body_emissive.jpeg": "body_emission.tga",
    "textures/body_specularGlossiness.png": "body_specular.tga",
    # flame textures
    "textures/flame_diffuse.png": "flame_diffuse.tga",
    "textures/flame_emissive.jpeg": "flame_emission.tga",
    # gas textures
    "textures/material_diffuse.jpeg": "gas_diffuse.tga",
    "textures/material_specularGlossiness.png": "gas_specular.tga",
}


def fix_flame_data(flame_data):
    start = flame_data.find("f 1/1/1 2/2/2 3/3/3")
    end = flame_data.find("f 458/458/458 459/459/459 460/460/460")
    return flame_data[:start] + flame_data[end:]


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    for mesh_index, filename in enumerate(OBJ_FILENAMES):
        if filename:
            obj_data = dump_obj_data(gltf, buffer, mesh_index)
            if filename == "flame.obj":
                obj_data = fix_flame_data(obj_data)
            filepath = os.path.join(DST_DIRECTORY, filename)
            with open(filepath, "w") as f:
                f.write(obj_data)


def linear_to_srgb(image):
    lookup_table = [pow(x / 255.0, 1 / 2.2) * 255 for x in range(256)]
    return image.point(lookup_table)


def fix_spec_image(image):
    image = Image.fromarray(numpy.amax(numpy.array(image), axis=2))
    return linear_to_srgb(image)


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
    for old_filename, tga_filename in IMG_FILENAMES.items():
        image = load_image(zip_file, old_filename)
        if "specularGlossiness" in old_filename:
            image = fix_spec_image(image)
        save_image(image, tga_filename)


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
