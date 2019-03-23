"""Preprocess the Junkrat model

The model is available for download from
    https://sketchfab.com/models/7deb2dd552df4bf4bb65005018176647

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile
from PIL import Image
from utils.gltf import dump_ani_data, dump_obj_data

SRC_FILENAME = "junkrat.zip"
DST_DIRECTORY = "../assets/junkrat"

IMG_FILENAMES = {
    "back": [
        "textures/Material_20_baseColor.png",
        "textures/Material_20_metallicRoughness.png",
    ],
    "head": [
        "textures/Material_34_baseColor.png",
        "textures/Material_34_metallicRoughness.png",
    ],
    "lower": [
        "textures/Material_19_baseColor.png",
        "textures/Material_19_metallicRoughness.png",
    ],
    "upper": [
        "textures/Material_22_baseColor.png",
        "textures/Material_22_metallicRoughness.png",
    ],
}

DEL_FILENAMES = [
    "back_roughness.tga",
    "head_metalness.tga",
    "lower_roughness.tga",
    "upper_roughness.tga",
]


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    for mesh_index in range(len(gltf["meshes"])):
        obj_data = dump_obj_data(gltf, buffer, mesh_index, with_skin=True)
        obj_filename = "junkrat{}.obj".format(mesh_index)
        obj_filepath = os.path.join(DST_DIRECTORY, obj_filename)
        with open(obj_filepath, "w") as f:
            f.write(obj_data)

    ani_data = dump_ani_data(gltf, buffer)
    ani_filepath = os.path.join(DST_DIRECTORY, "junkrat.ani")
    with open(ani_filepath, "w") as f:
        f.write(ani_data)


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
    for name, (basecolor_path, packed_path) in IMG_FILENAMES.items():
        basecolor_image = load_image(zip_file, basecolor_path)
        save_image(basecolor_image, "{}_basecolor.tga".format(name))

        packed_image = load_image(zip_file, packed_path)
        _, roughness_image, metalness_image = packed_image.split()
        save_image(roughness_image, "{}_roughness.tga".format(name))
        save_image(metalness_image, "{}_metalness.tga".format(name))

    for del_filename in DEL_FILENAMES:
        del_filepath = os.path.join(DST_DIRECTORY, del_filename)
        os.remove(del_filepath)


def print_mesh2material(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    meshes = gltf["meshes"]

    mesh2material = []
    for mesh in meshes:
        assert len(mesh["primitives"]) == 1
        primitive = mesh["primitives"][0]
        mesh2material.append(primitive["material"])

    num_meshes = len(meshes)
    chunk_size = (num_meshes + 2) / 3
    print("    int mesh2material[{}] = {{".format(num_meshes))
    for i in range(0, num_meshes, chunk_size):
        indices = [str(j) for j in mesh2material[i:(i + chunk_size)]]
        print("        {}".format(", ".join(indices) + ","))
    print("    };")


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)
        # print_mesh2material(zip_file)


if __name__ == "__main__":
    main()
