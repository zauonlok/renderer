"""Preprocess the Mech Drone model

The model is available for download from
    https://sketchfab.com/models/8d06874aac5246c59edb4adbe3606e0e

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile
from PIL import Image
import utils

SRC_FILENAME = "mech_drone.zip"
DST_DIRECTORY = "../assets/drone"

OBJ_FILENAME = "drone.obj"

DIFFUSE_FILENAME = "textures/Robot_diffuse.jpeg"
NORMAL_FILENAME = "textures/Robot_normal.jpeg"
OCCLUSION_FILENAME = "textures/Robot_occlusion.jpeg"
EMISSIVE_FILENAME = "textures/Robot_emissive.jpeg"
PACKED_FILENAME = "textures/Robot_specularGlossiness.png"


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    mesh = utils.load_gltf_mesh(gltf, buffer, gltf["meshes"][0])
    obj_data, tan_data = utils.dump_mesh_data(mesh)

    obj_filepath = os.path.join(DST_DIRECTORY, OBJ_FILENAME)
    with open(obj_filepath, "w") as f:
        f.write(obj_data)

    tan_filepath = obj_filepath[:-4] + ".tan"
    with open(tan_filepath, "w") as f:
        f.write(tan_data)


def load_image(zip_file, filename):
    with zip_file.open(filename) as f:
        image = Image.open(f)
        image = image.transpose(Image.FLIP_TOP_BOTTOM)
        return image


def save_image(image, filename, size=(512, 512)):
    image = image.resize(size, Image.LANCZOS)
    filepath = os.path.join(DST_DIRECTORY, filename)
    image.save(filepath, rle=True)


def process_images(zip_file):
    diffuse_image = load_image(zip_file, DIFFUSE_FILENAME)
    save_image(diffuse_image, "diffuse.tga")

    normal_image = load_image(zip_file, NORMAL_FILENAME)
    save_image(normal_image, "normal.tga", size=(1024, 1024))

    occlusion_image = load_image(zip_file, OCCLUSION_FILENAME)
    occlusion_image, _, _ = occlusion_image.split()
    save_image(occlusion_image, "occlusion.tga")

    emissive_image = load_image(zip_file, EMISSIVE_FILENAME)
    save_image(emissive_image, "emissive.tga")

    packed_image = load_image(zip_file, PACKED_FILENAME)
    packed_bands = packed_image.split()

    specular_image = Image.merge("RGB", packed_bands[:3])
    glossiness_image = packed_bands[3]
    save_image(specular_image, "specular.tga")
    save_image(glossiness_image, "glossiness.tga")


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
