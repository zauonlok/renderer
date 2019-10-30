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

import numpy
from PIL import Image

from .utils.gltf import dump_obj_data

SRC_FILENAME = "centaur.zip"
DST_DIRECTORY = "../assets/centaur"

OBJ_FILENAMES = [
    "body.obj",
    None,
    "flame.obj",
    "gas.obj",
]

IMG_FILENAMES = {
    "body": [
        "textures/body_diffuse.jpeg",
        "textures/body_emissive.jpeg",
        "textures/body_specularGlossiness.png",
    ],
    "flame": [
        "textures/flame_diffuse.png",
        "textures/flame_emissive.jpeg",
        None,
    ],
    "gas": [
        "textures/material_diffuse.jpeg",
        None,
        "textures/material_specularGlossiness.png",
    ],
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


def fix_spec_image(image):
    array = numpy.amax(numpy.array(image), axis=2) / 255.0
    array = numpy.uint8(numpy.power(array, 1 / 2.2) * 255)
    return Image.fromarray(array)


def process_images(zip_file):
    for name, paths in IMG_FILENAMES.items():
        diffuse_path, emission_path, specular_path = paths

        if diffuse_path:
            diffuse_image = load_image(zip_file, diffuse_path)
            save_image(diffuse_image, "{}_diffuse.tga".format(name))

        if emission_path:
            emission_image = load_image(zip_file, emission_path)
            save_image(emission_image, "{}_emission.tga".format(name))

        if specular_path:
            specular_image = load_image(zip_file, specular_path)
            specular_image = fix_spec_image(specular_image)
            save_image(specular_image, "{}_specular.tga".format(name))


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
