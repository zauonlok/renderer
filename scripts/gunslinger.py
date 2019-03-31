"""Preprocess the Gunslinger model

The model is available for download from
    https://sketchfab.com/models/ffdf2f4194954bef836b4aa0468062f7

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile
from PIL import Image
from utils.gltf import dump_obj_data

SRC_FILENAME = "gunslinger.zip"
DST_DIRECTORY = "../assets/gunslinger"

OBJ_FILENAMES = [
    "gunslinger.obj",
    "revolver.obj",
]

IMG_FILENAMES = {
    "gunslinger": [
        "textures/gunslinger_diffuse.png",
        "textures/gunslinger_specularGlossiness.png",
    ],
    "revolver": [
        "textures/revolver_diffuse.png",
        "textures/revolver_specularGlossiness.png",
    ],
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


def save_image(image, filename):
    image = image.resize((512, 512), Image.LANCZOS)
    filepath = os.path.join(DST_DIRECTORY, filename)
    image.save(filepath, rle=True)


def process_images(zip_file):
    for name, (diffuse_path, packed_path) in IMG_FILENAMES.items():
        diffuse_image = load_image(zip_file, diffuse_path)
        save_image(diffuse_image, "{}_diffuse.tga".format(name))

        packed_image = load_image(zip_file, packed_path)
        if name == "gunslinger":
            packed_bands = packed_image.split()
            specular_image = packed_bands[0]
            save_image(specular_image, "{}_specular.tga".format(name))
            glossiness_image = packed_bands[3]
            save_image(glossiness_image, "{}_glossiness.tga".format(name))
        else:
            specular_image = packed_image
            save_image(specular_image, "{}_specular.tga".format(name))


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
