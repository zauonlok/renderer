"""Preprocess the Dancing Crab model

The model is available for download from
    https://sketchfab.com/models/280863886fee409ab3c8168f07caa89f

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile

from PIL import Image

from .utils.gltf import dump_obj_data, dump_skin_ani_data

SRC_FILENAME = "dancing_crab_-_uca_mjoebergi.zip"
DST_DIRECTORY = "../assets/crab"


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    obj_data = dump_obj_data(gltf, buffer, 0, with_skin=True, with_tangent=True)
    obj_filepath = os.path.join(DST_DIRECTORY, "crab.obj")
    with open(obj_filepath, "w") as f:
        f.write(obj_data)

    ani_data = dump_skin_ani_data(gltf, buffer)
    ani_filepath = os.path.join(DST_DIRECTORY, "crab.ani")
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
    diffuse_path = "textures/FullyAssembledinitialShadingGroup1_diffuse.jpeg"
    packed_path = "textures/FullyAssembledinitialShadingGroup1_specularGlossiness.png"
    normal_path = "textures/FullyAssembledinitialShadingGroup1_normal.png"

    diffuse_image = load_image(zip_file, diffuse_path)
    save_image(diffuse_image, "crab_diffuse.tga")

    packed_image = load_image(zip_file, packed_path)
    packed_bands = packed_image.split()

    specular_image = Image.merge("RGB", packed_bands[:3])
    save_image(specular_image, "crab_specular.tga")

    glossiness_image = packed_bands[3]
    save_image(glossiness_image, "crab_glossiness.tga")

    normal_image = load_image(zip_file, normal_path)
    save_image(normal_image, "crab_normal.tga")


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
