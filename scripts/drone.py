"""Preprocess the Mech Drone model

The model is available for download from
    https://sketchfab.com/models/8d06874aac5246c59edb4adbe3606e0e

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile

from PIL import Image

from .utils.gltf import dump_node_ani_data, dump_obj_data, dump_skin_ani_data

SRC_FILENAME = "mech_drone.zip"
DST_DIRECTORY = "../assets/drone"

IMG_FILENAMES = {
    "drone": [
        "textures/Robot_diffuse.jpeg",
        "textures/Robot_emissive.jpeg",
        "textures/Robot_occlusion.jpeg",
        "textures/Robot_normal.jpeg",
        "textures/Robot_specularGlossiness.png",
    ],
    "fire": [
        "textures/Fire_diffuse.png",
        "textures/Fire_emissive.jpeg",
        None,
        None,
        None,
    ],
}


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")

    drone_obj_data = dump_obj_data(
        gltf, buffer, 0, with_skin=True, with_tangent=True
    )
    drone_filepath = os.path.join(DST_DIRECTORY, "drone.obj")
    with open(drone_filepath, "w") as f:
        f.write(drone_obj_data)

    for mesh_index in range(1, len(gltf["meshes"])):
        fire_obj_data = dump_obj_data(gltf, buffer, mesh_index)
        fire_filename = "fire{}.obj".format(mesh_index - 1)
        fire_filepath = os.path.join(DST_DIRECTORY, fire_filename)
        with open(fire_filepath, "w") as f:
            f.write(fire_obj_data)

    drone_ani_data = dump_skin_ani_data(gltf, buffer)
    drone_ani_filepath = os.path.join(DST_DIRECTORY, "drone.ani")
    with open(drone_ani_filepath, "w") as f:
        f.write(drone_ani_data)

    fire_ani_data, _ = dump_node_ani_data(gltf, buffer)
    fire_ani_filepath = os.path.join(DST_DIRECTORY, "fire.ani")
    with open(fire_ani_filepath, "w") as f:
        f.write(fire_ani_data)


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
        diffuse_path, emission_path = paths[:2]
        occlusion_path, normal_path, packed_path = paths[2:]

        if diffuse_path:
            diffuse_image = load_image(zip_file, diffuse_path)
            save_image(diffuse_image, "{}_diffuse.tga".format(name))

        if emission_path:
            emission_image = load_image(zip_file, emission_path)
            save_image(emission_image, "{}_emission.tga".format(name))

        if occlusion_path:
            occlusion_image = load_image(zip_file, occlusion_path)
            occlusion_image = occlusion_image.split()[0]
            save_image(occlusion_image, "{}_occlusion.tga".format(name))

        if normal_path:
            normal_image = load_image(zip_file, normal_path)
            save_image(normal_image, "{}_normal.tga".format(name))

        if packed_path:
            packed_image = load_image(zip_file, packed_path)
            packed_bands = packed_image.split()
            specular_image = Image.merge("RGB", packed_bands[:3])
            glossiness_image = packed_bands[3]
            save_image(specular_image, "{}_specular.tga".format(name))
            save_image(glossiness_image, "{}_glossiness.tga".format(name))


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        process_meshes(zip_file)
        process_images(zip_file)


if __name__ == "__main__":
    main()
