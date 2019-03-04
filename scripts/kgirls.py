"""Preprocess the Kgirls model

The model is available for download from
    https://sketchfab.com/models/d2f946f58a8040ae993cda70c97b302c

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile
from PIL import Image
from utils.gltf import dump_ani_data, dump_obj_data

SRC_FILENAME = "kgirls01.zip"
DST_DIRECTORY = "../assets/kgirls"

OBJ_FILENAMES = [
    "body.obj",
    "hair.obj",
    "face.obj",
    "pupils.obj",
]

IMG_FILENAMES = {
    "textures/01_-_Default_baseColor.png": "kgirls.tga",
}


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    for mesh_index, obj_filename in enumerate(OBJ_FILENAMES):
        obj_data = dump_obj_data(gltf, buffer, mesh_index, with_skin=True)
        obj_filepath = os.path.join(DST_DIRECTORY, obj_filename)
        with open(obj_filepath, "w") as f:
            f.write(obj_data)

    ani_data = dump_ani_data(gltf, buffer)
    ani_filepath = os.path.join(DST_DIRECTORY, "kgirls.ani")
    with open(ani_filepath, "w") as f:
            f.write(ani_data)


def process_images(zip_file):
    for old_filename, tga_filename in IMG_FILENAMES.items():
        with zip_file.open(old_filename) as f:
            image = Image.open(f)
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
