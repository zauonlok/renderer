"""Preprocess the Noble Craftsman model

The model is available for download from
    https://sketchfab.com/models/0e8ff87ffaa24731b2474badebca870d

The Python Imaging Library is required
    pip install pillow
"""

import json
import os
import zipfile
from PIL import Image
import utils

SRC_FILENAME = "the_noble_craftsman.zip"
DST_DIRECTORY = "../assets/craftsman"

OBJ_FILENAMES = [
    "anvil.obj",
    "smith.obj",
    "floor.obj",
    "shoulderpad0.obj",
    "hammer.obj",
    "hotiron.obj",
    "shoulderpad1.obj",
    "spark.obj",
]

IMG_FILENAMES = {
    "textures/02_-_Default_baseColor.png": "smith_basecolor.tga",
    "textures/02_-_Default_emissive.jpg": "smith_emissive.tga",
    "textures/03_-_Default_baseColor.png": "floor_basecolor.tga",
    "textures/08_-_Default_baseColor.png": "spark_basecolor.tga",
    "textures/08_-_Default_emissive.jpg": "spark_emissive.tga",
    "textures/09_-_Default_baseColor.jpg": "anvil_basecolor.tga",
}


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")
    assert len(buffer) == 355248

    meshes = []
    for mesh in gltf["meshes"]:
        mesh = utils.load_gltf_mesh(gltf, buffer, mesh)
        meshes.append(mesh)

    # there are 20 identical spark models, just save the first one
    assert len(meshes) == 27
    assert all([x == meshes[7] for x in meshes[8:]])

    for filename, mesh in zip(OBJ_FILENAMES, meshes):
        content = utils.dump_mesh_data(mesh)
        filepath = os.path.join(DST_DIRECTORY, filename)
        with open(filepath, "w") as f:
            f.write(content)

    utils.print_mesh_materials(gltf)
    utils.print_gltf_transforms(gltf)


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
