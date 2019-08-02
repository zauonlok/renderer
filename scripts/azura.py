"""Preprocess the Azura model

The model is available for download from
    https://sketchfab.com/models/bf3b946f6dbb448eb25811e87e006f08

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile

from PIL import Image

from utils.gltf import dump_obj_data

SRC_FILENAME = "dancing_girl_from_fire_emblem_if.zip"
DST_DIRECTORY = "../assets/azura"

OBJ_FILENAMES = [
    "bijou0.obj",
    "ground.obj",
    "body.obj",
    "robe0.obj",
    "robe1.obj",
    "head.obj",
    "hair.obj",
    "motif0.obj",
    "yeux0.obj",
    "yeux1.obj",
    "yeux2.obj",
    "yeux3.obj",
    "bijou1.obj",
    "laniere.obj",
    "motif1.obj",
    "cuissarde0.obj",
    "motif2.obj",
    "cuissarde1.obj",
    "bijou2.obj",
]

IMG_FILENAMES = {
    "textures/bijouxOpac_mat_baseColor.png": "bijoux_diffuse.tga",
    "textures/body_mat_baseColor.png": "body_diffuse.tga",
    "textures/cuissarde_mat_baseColor.png": "cuissarde_diffuse.tga",
    "textures/ground_mat_baseColor.png": "ground_diffuse.tga",
    "textures/hair_mat_baseColor.png": "hair_diffuse.tga",
    "textures/head_mat_baseColor.png": "head_diffuse.tga",
    "textures/laniere_mat_baseColor.png": "laniere_diffuse.tga",
    "textures/motif_left_mat_baseColor.png": "motif_diffuse.tga",
    "textures/robeEndroit_mat_baseColor.png": "robe_diffuse.tga",
    "textures/yeux_mat_baseColor.png": "yeux_diffuse.tga",
}


def process_meshes(zip_file):
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
