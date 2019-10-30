"""Preprocess the Whip of the Icestorm model

The model is available for download from
    https://sketchfab.com/models/46ed142dc5e94c048b1f09c24bf74c95

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile

from PIL import Image

from .utils.gltf import dump_node_ani_data, dump_obj_data

SRC_FILENAME = "ethiziel_whip_of_the_icestorm.zip"
DST_DIRECTORY = "../assets/whip"

IMG_FILENAMES = {
    "textures/Material_70_baseColor.jpg": "body_diffuse.tga",
    "textures/Material_72_baseColor.png": "lash_diffuse.tga",
    "textures/Material_72_emissive.jpeg": "lash_emission.tga",
}


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    for mesh_index in range(len(gltf["meshes"])):
        obj_data = dump_obj_data(gltf, buffer, mesh_index)
        obj_filename = "whip{}.obj".format(mesh_index)
        obj_filepath = os.path.join(DST_DIRECTORY, obj_filename)
        with open(obj_filepath, "w") as f:
            f.write(obj_data)

    ani_data, _ = dump_node_ani_data(gltf, buffer)
    ani_filepath = os.path.join(DST_DIRECTORY, "whip.ani")
    with open(ani_filepath, "w") as f:
        f.write(ani_data)


def load_image(zip_file, filename):
    with zip_file.open(filename) as f:
        image = Image.open(f)
        image = image.transpose(Image.FLIP_TOP_BOTTOM)
        return image


def save_image(image, filename, size=1024):
    if max(image.size) > size:
        image = image.resize((size, size), Image.LANCZOS)
    filepath = os.path.join(DST_DIRECTORY, filename)
    image.save(filepath, rle=True)


def process_images(zip_file):
    for old_filename, tga_filename in IMG_FILENAMES.items():
        image = load_image(zip_file, old_filename)
        if image.mode == "P":
            image = image.convert("RGBA")
        save_image(image, tga_filename)


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
