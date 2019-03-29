"""Preprocess the Papermill Ruins cubemaps

The cubemaps are available for download from
    https://github.com/KhronosGroup/glTF-WebGL-PBR/archive/master.zip

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import os
import zipfile
from PIL import Image

SRC_FILENAME = "glTF-WebGL-PBR-master.zip"
DST_DIRECTORY = "../assets/common/papermill"


def linear_to_srgb(image):
    lookup_table = [pow(x / 255.0, 1 / 2.2) * 255 for x in range(256)] * 3
    return image.point(lookup_table)


def save_image(zip_file, old_filepath, new_filename):
    new_filepath = os.path.join(DST_DIRECTORY, new_filename)
    with zip_file.open(old_filepath) as f:
        image = Image.open(f)
        image = linear_to_srgb(image)
        image.save(new_filepath, rle=True)


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        for old_filepath in zip_file.namelist():
            if "papermill/diffuse/diffuse_" in old_filepath:
                old_filename = os.path.basename(old_filepath)
                new_filename = old_filename[:-4] + ".tga"
                save_image(zip_file, old_filepath, new_filename)
            elif "papermill/specular/specular_" in old_filepath:
                old_filename = os.path.basename(old_filepath)
                new_filename = old_filename[:-4] + ".tga"
                save_image(zip_file, old_filepath, new_filename)
            elif "papermill/environment/environment_" in old_filepath:
                old_filename = os.path.basename(old_filepath)
                new_filename = "skybox_" + old_filename[12:-6] + ".tga"
                save_image(zip_file, old_filepath, new_filename)


if __name__ == "__main__":
    main()
