"""Preprocess the Papermill Ruins envmaps

The envmaps are available for download from
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


def gamma_correct(image):
    lookup_table = [pow(x / 255.0, 1 / 2.2) * 255 for x in range(256)] * 3
    return image.point(lookup_table)


def save_image(zip_file, old_filepath):
    new_filename = os.path.basename(os.path.splitext(old_filepath)[0]) + ".tga"
    new_filepath = os.path.join(DST_DIRECTORY, new_filename)
    with zip_file.open(old_filepath) as f:
        image = Image.open(f)
        image = gamma_correct(image)
        image.save(new_filepath, rle=True)


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        for filepath in zip_file.namelist():
            if "papermill/diffuse/diffuse_" in filepath:
                save_image(zip_file, filepath)
            elif "papermill/specular/specular_" in filepath:
                save_image(zip_file, filepath)


if __name__ == "__main__":
    main()
