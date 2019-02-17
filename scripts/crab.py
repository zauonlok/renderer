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
import utils

SRC_FILENAME = "dancing_crab_-_uca_mjoebergi.zip"
DST_DIRECTORY = "../assets/crab"

OBJ_FILENAME = "crab.obj"

DIFFUSE_FILENAME = "textures/FullyAssembledinitialShadingGroup1_diffuse.jpeg"
NORMAL_FILENAME = "textures/FullyAssembledinitialShadingGroup1_normal.png"
PACKED_FILENAME = "textures/FullyAssembledinitialShadingGroup1_specularGlossiness.png"


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    mesh = utils.load_gltf_mesh(gltf, buffer, gltf["meshes"][0])
    obj_data, tan_data = utils.dump_mesh_data(mesh)

    obj_filepath = os.path.join(DST_DIRECTORY, OBJ_FILENAME)
    with open(obj_filepath, "w") as f:
        f.write(obj_data)

    tan_filepath = os.path.join(DST_DIRECTORY, OBJ_FILENAME[:-4] + ".tan")
    with open(tan_filepath, "w") as f:
        f.write(tan_data)


def load_image(zip_file, filename, rgb_only):
    with zip_file.open(filename) as f:
        image = Image.open(f)
        if rgb_only:
            bands = image.split()
            image = Image.merge("RGB", bands[:3])
        image = image.transpose(Image.FLIP_TOP_BOTTOM)
        return image


def process_images(zip_file):
    diffuse_image = load_image(zip_file, DIFFUSE_FILENAME, True)
    diffuse_image = diffuse_image.resize((512, 512), Image.LANCZOS)
    diffuse_filepath = os.path.join(DST_DIRECTORY, "diffuse.tga")
    diffuse_image.save(diffuse_filepath, rle=True)

    normal_image = load_image(zip_file, NORMAL_FILENAME, True)
    normal_filepath = os.path.join(DST_DIRECTORY, "normal.tga")
    normal_image.save(normal_filepath, rle=True)

    packed_image = load_image(zip_file, PACKED_FILENAME, False)
    packed_bands = packed_image.split()

    specular_image = Image.merge("RGB", packed_bands[:3])
    specular_image = specular_image.resize((512, 512), Image.LANCZOS)
    specular_filepath = os.path.join(DST_DIRECTORY, "specular.tga")
    specular_image.save(specular_filepath, rle=True)

    glossiness_image = packed_bands[3]
    glossiness_image = glossiness_image.resize((512, 512), Image.LANCZOS)
    glossiness_filepath = os.path.join(DST_DIRECTORY, "glossiness.tga")
    glossiness_image.save(glossiness_filepath, rle=True)


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
