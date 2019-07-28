"""Preprocess the McCree model

The model is available for download from
    https://sketchfab.com/models/38aedc02c0b2412babdc4d0eac7c6803
"""

from __future__ import print_function

import json
import os
import zipfile

from utils.gltf import dump_obj_data

SRC_FILENAME = "low_poly_mccree.zip"
DST_DIRECTORY = "../assets/mccree"


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    for mesh_index in range(len(gltf["meshes"])):
        obj_data = dump_obj_data(gltf, buffer, mesh_index)
        filename = "mccree{}.obj".format(mesh_index)
        filepath = os.path.join(DST_DIRECTORY, filename)
        with open(filepath, "w") as f:
            f.write(obj_data)


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)


if __name__ == "__main__":
    main()
