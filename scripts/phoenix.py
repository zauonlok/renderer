"""Preprocess the Phoenix Bird model

The model is available for download from
    https://sketchfab.com/models/844ba0cf144a413ea92c779f18912042

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile

from PIL import Image

from utils.gltf import dump_obj_data, dump_skin_ani_data

SRC_FILENAME = "phoenix_bird.zip"
DST_DIRECTORY = "../assets/phoenix"

OBJ_FILENAMES = [
    "body.obj",
    "wings.obj",
]

IMG_FILENAMES = {
    "textures/MatI_Ride_FengHuang_01a_baseColor.png": "body_diffuse.tga",
    "textures/MatI_Ride_FengHuang_01a_emissive.png": "body_emission.tga",
    "textures/MatI_Ride_FengHuang_01b_baseColor.png": "wings_diffuse.tga",
    "textures/MatI_Ride_FengHuang_01b_emissive.png": "wings_emission.tga",
}


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    for mesh_index, obj_filename in enumerate(OBJ_FILENAMES):
        obj_data = dump_obj_data(gltf, buffer, mesh_index, with_skin=True)
        obj_filepath = os.path.join(DST_DIRECTORY, obj_filename)
        with open(obj_filepath, "w") as f:
            f.write(obj_data)

    ani_data = dump_skin_ani_data(gltf, buffer)
    ani_filepath = os.path.join(DST_DIRECTORY, "phoenix.ani")
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
    for old_filename, tga_filename in IMG_FILENAMES.items():
        image = load_image(zip_file, old_filename)
        if "emissive" in old_filename:
            image = Image.merge("RGB", image.split()[:3])
        save_image(image, tga_filename)


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
