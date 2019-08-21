"""Preprocess the Buster Drone model

The model is available for download from
    https://sketchfab.com/models/294e79652f494130ad2ab00a13fdbafd

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile

from PIL import Image

from utils.gltf import dump_node_ani_data, dump_obj_data

SRC_FILENAME = "buster_drone.zip"
DST_DIRECTORY = "../assets/buster"

IMG_FILENAMES = {
    "body": [
        "textures/body_baseColor.png",
        "textures/body_metallicRoughness.png",
        "textures/body_normal.png",
        "textures/body_emissive.png",
    ],
    "legs": [
        "textures/material_baseColor.png",
        "textures/material_metallicRoughness.png",
        "textures/material_normal.png",
        None,
    ],
}


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    for mesh_index in range(1, len(gltf["meshes"])):
        obj_data = dump_obj_data(gltf, buffer, mesh_index, with_tangent=True)
        obj_filename = "buster{}.obj".format(mesh_index - 1)
        obj_filepath = os.path.join(DST_DIRECTORY, obj_filename)
        with open(obj_filepath, "w") as f:
            f.write(obj_data)

    ani_data, _ = dump_node_ani_data(gltf, buffer)
    ani_filepath = os.path.join(DST_DIRECTORY, "buster.ani")
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
    for name, paths in IMG_FILENAMES.items():
        basecolor_path, packed_path, normal_path, emission_path = paths

        if basecolor_path:
            basecolor_image = load_image(zip_file, basecolor_path)
            save_image(basecolor_image, "{}_basecolor.tga".format(name))

        if packed_path:
            packed_image = load_image(zip_file, packed_path)
            packed_bands = packed_image.split()
            occlusion_image, roughness_image, metalness_image = packed_bands
            save_image(occlusion_image, "{}_occlusion.tga".format(name))
            save_image(roughness_image, "{}_roughness.tga".format(name))
            save_image(metalness_image, "{}_metalness.tga".format(name))

        if normal_path:
            normal_image = load_image(zip_file, normal_path)
            save_image(normal_image, "{}_normal.tga".format(name))

        if emission_path:
            emission_image = load_image(zip_file, emission_path)
            save_image(emission_image, "{}_emission.tga".format(name))


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
