"""Preprocess the McCree model

The model is available for download from
    https://sketchfab.com/models/38aedc02c0b2412babdc4d0eac7c6803
"""

import json
import os
import zipfile
import utils

SRC_FILENAME = "low_poly_mccree.zip"
DST_DIRECTORY = "../assets/mccree"


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")
    assert len(buffer) == 299192

    meshes = []
    for mesh in gltf["meshes"]:
        mesh = utils.load_gltf_mesh(gltf, buffer, mesh)
        meshes.append(mesh)

    for index, mesh in enumerate(meshes):
        content = utils.dump_mesh_data(mesh)
        filename = "part{}.obj".format(index)
        filepath = os.path.join(DST_DIRECTORY, filename)
        with open(filepath, "w") as f:
            f.write(content)

    utils.print_mesh_materials(gltf)
    utils.print_gltf_transforms(gltf)


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)


if __name__ == "__main__":
    main()
