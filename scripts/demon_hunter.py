"""Preprocess the Demon Hunter model

The model is available for download from
    https://sketchfab.com/models/ff6a14371bc347d9a1526f5e64b29327

The Python Imaging Library is required
    pip install pillow
"""

import json
import os
import zipfile
from PIL import Image
import utils

SRC_FILENAME = "demon_hunter.zip"
DST_DIRECTORY = "../assets/demon_hunter"

OBJ_FILENAMES = [
    "body.obj",
    # there are 2 identical crossbow models
    "crossbow.obj",
    "crossbow.obj",
    # there are 2 identical eyeball models
    "eye.obj",
    "eye.obj",
    "floor.obj",
]

IMG_FILENAMES = {
    "textures/DH_crossbow_phong_baseColor.png": "crossbow.tga",
    "textures/DH_matl1_baseColor.png": "body.tga",
}


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")
    assert len(buffer) == 445304

    meshes = []
    for mesh in gltf["meshes"]:
        mesh = utils.load_gltf_mesh(gltf, buffer, mesh)
        meshes.append(mesh)

    assert len(OBJ_FILENAMES) == len(meshes)
    for filename, mesh in zip(OBJ_FILENAMES, meshes):
        content = utils.dump_mesh_data(mesh)
        filepath = os.path.join(DST_DIRECTORY, filename)
        with open(filepath, "w") as f:
            f.write(content)


def process_images(zip_file):
    for old_filename, tga_filename in IMG_FILENAMES.items():
        with zip_file.open(old_filename) as f:
            image = Image.open(f)
            bands = image.split()
            assert len(bands) in [3, 4]
            image = Image.merge("RGB", bands[:3])
            image = image.transpose(Image.FLIP_TOP_BOTTOM)
            image = image.resize((512, 512), Image.LANCZOS)
            filepath = os.path.join(DST_DIRECTORY, tga_filename)
            image.save(filepath, rle=True)


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
