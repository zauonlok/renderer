"""Preprocess the Squid Ink Bottle model

The model is available for download from
    https://sketchfab.com/models/df4068d6e911479ca02e313483cb273d

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile
from PIL import Image
from utils.gltf import dump_obj_data, load_node_data

SRC_FILENAME = "squid_ink_bottle.zip"
DST_DIRECTORY = "../assets/squid"

OBJ_FILENAMES = [
    "calamar0.obj",
    "calamar1.obj",
    "bottle.obj",
    "floor.obj",
    "drop.obj",
]

IMG_FILENAMES = {
    "calamar": [
        "textures/Calamar_baseColor.jpeg",
        "textures/Calamar_metallicRoughness.png",
    ],
    "bottle": [
        "textures/Bottle_Floor_baseColor.png",
        "textures/Bottle_Floor_metallicRoughness.png",
    ],
}


def process_mesh(zip_file):
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


def save_image(image, filename):
    image = image.resize((512, 512), Image.LANCZOS)
    filepath = os.path.join(DST_DIRECTORY, filename)
    image.save(filepath, rle=True)


def process_images(zip_file):
    for name, (basecolor_path, packed_path) in IMG_FILENAMES.items():
        basecolor_image = load_image(zip_file, basecolor_path)
        save_image(basecolor_image, "{}_basecolor.tga".format(name))

        packed_image = load_image(zip_file, packed_path)
        occlusion_image, roughness_image, _ = packed_image.split()
        save_image(occlusion_image, "{}_occlusion.tga".format(name))
        save_image(roughness_image, "{}_roughness.tga".format(name))


def print_transforms(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    nodes = load_node_data(gltf)
    nodes = [node for node in nodes if node.mesh is not None]
    transforms = [node.world_transform for node in nodes]

    row_pattern = "            {{{:9.6f}f, {:9.6f}f, {:9.6f}f, {:9.6f}f}},"
    print("    mat4_t transforms[{}] = {{".format(len(transforms)))
    for transform in transforms:
        print("        {{")
        for i in range(4):
            print(row_pattern.format(*transform.data[i]))
        print("        }},")
    print("    };")


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_mesh(zip_file)
        process_images(zip_file)
        # print_transforms(zip_file)


if __name__ == "__main__":
    main()
