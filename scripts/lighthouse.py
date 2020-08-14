"""Preprocess the Lighthouse model

The model is available for download from
    https://sketchfab.com/models/1a85945dd2a840f594bf6cb003176a54

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile

from PIL import Image

from utils.gltf import dump_obj_data

SRC_FILENAME = "the_lighthouse.zip"
DST_DIRECTORY = "../assets/lighthouse"

IMG_FILENAMES = {
    "textures/Material_343_baseColor.jpg": "lighthouse0.tga",
    "textures/Material_185_baseColor.jpg": "lighthouse1.tga",
    "textures/Material_186_baseColor.jpg": "lighthouse2.tga",
    "textures/Material_144_baseColor.jpg": "lighthouse3.tga",
    "textures/Material_371_baseColor.png": "lighthouse4.tga",
    "textures/Material_286_baseColor.jpg": "lighthouse5.tga",
    "textures/Material_221_baseColor.jpg": "lighthouse6.tga",
    "textures/Material_287_baseColor.jpg": "lighthouse7.tga",
    "textures/Material_289_baseColor.jpg": "lighthouse8.tga",
    "textures/Material_369_baseColor.jpg": "lighthouse9.tga",
    "textures/22_-_Default_baseColor.png": "lighthouse10.tga",
    "textures/Material_139_baseColor.jpg": "lighthouse11.tga",
    "textures/Material_138_baseColor.jpg": "lighthouse12.tga",
    "textures/Material_346_baseColor.jpg": "lighthouse13.tga",
    "textures/Material_213_baseColor.jpg": "lighthouse14.tga",
    "textures/Material_319_baseColor.jpg": "lighthouse15.tga",
    "textures/Material_188_baseColor.jpg": "lighthouse16.tga",
    "textures/Material_211_baseColor.jpg": "lighthouse17.tga",
    "textures/Material_134_baseColor.jpg": "lighthouse18.tga",
    "textures/Material_312_baseColor.jpg": "lighthouse19.tga",
    "textures/Material_210_baseColor.jpg": "lighthouse20.tga",
    "textures/Material_214_baseColor.jpg": "lighthouse21.tga",
    "textures/Material_216_baseColor.jpg": "lighthouse22.tga",
    "textures/Material_347_baseColor.jpg": "lighthouse23.tga",
    "textures/Material_215_baseColor.jpg": "lighthouse24.tga",
    "textures/Material_8721_baseColor.jpg": "lighthouse25.tga",
    "textures/Material_263_baseColor.jpg": "lighthouse26.tga",
    "textures/Material_342_baseColor.png": "lighthouse27.tga",
}


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    for mesh_index in range(len(gltf["meshes"])):
        obj_data = dump_obj_data(gltf, buffer, mesh_index)
        filename = "lighthouse{}.obj".format(mesh_index)
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


def process_images(zip_file):
    for old_filename, tga_filename in IMG_FILENAMES.items():
        image = load_image(zip_file, old_filename)
        save_image(image, tga_filename)


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
