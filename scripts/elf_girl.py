"""Preprocess the Elf Girl model

The model is available for download from
    https://sketchfab.com/models/52f2e84961b94760b7805c178890d644

The Python Imaging Library is required
    pip install pillow
"""

import json
import os
import zipfile
from PIL import Image
import utils

SRC_FILENAME = "elf_girl.zip"
DST_DIRECTORY = "../assets/elf_girl"

OBJ_FILENAMES = [
    "face0.obj",
    "face1.obj",
    "body0.obj",
    "body1.obj",
    "body2.obj",
    "hair.obj",
    "ce.obj",
]

IMG_FILENAMES = {
    "textures/Rig2lambert23SG_baseColor.png": "face.tga",
    "textures/lambert22SG_baseColor.png": "body.tga",
    "textures/lambert25SG_baseColor.png": "hair.tga",
    "textures/pasted__lambert2SG_baseColor.png": "ce.tga",
}


def fix_texcoords(mesh):
    texcoords = mesh["texcoords"]
    for i in range(len(texcoords)):
        u, v = texcoords[i]
        # the default wrap parameter is GL_REPEAT, which causes the integer part
        # of texcoords to be ignored, only the fractional part is used, see
        # http://docs.gl/gl2/glTexParameter
        texcoords[i] = [u % 1, v % 1]


def process_meshes(zip_file):
    gltf = json.loads(zip_file.read("scene.gltf"))
    buffer = zip_file.read("scene.bin")
    assert len(buffer) == 641280

    meshes = []
    for mesh in gltf["meshes"]:
        mesh = utils.load_gltf_mesh(gltf, buffer, mesh)
        fix_texcoords(mesh)
        meshes.append(mesh)

    assert len(OBJ_FILENAMES) == len(meshes)
    for filename, mesh in zip(OBJ_FILENAMES, meshes):
        content = utils.dump_mesh_data(mesh)
        filepath = os.path.join(DST_DIRECTORY, filename)
        with open(filepath, "w") as f:
            f.write(content)

    utils.print_mesh_materials(gltf)
    utils.print_gltf_transforms(gltf)


def process_images(zip_file):
    for png_filename, tga_filename in IMG_FILENAMES.items():
        with zip_file.open(png_filename) as f:
            image = Image.open(f)
            bands = image.split()
            assert len(bands) in [3, 4]
            image = Image.merge("RGB", bands[:3])
            # coordinates origin:
            #     lower left corner: OpenGL, TGA
            #     upper left corner: WebGL, glTF
            # references:
            #     https://github.com/KhronosGroup/glTF/issues/1021
            #     https://github.com/KhronosGroup/glTF-WebGL-PBR/issues/16
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
