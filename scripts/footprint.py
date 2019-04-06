"""Compress the Footprint Court cubemaps

The cubemaps are available for download from
    https://github.com/ux3d/Sample-Environments/archive/master.zip
"""

from __future__ import print_function

import os
import zipfile
from utils.hdr import load_hdr_image, dump_hdr_image

SRC_FILENAME = "Sample-Environments-master.zip"
DST_DIRECTORY = "../assets/common/footprint"


def compress_image(zip_file, old_filepath):
    old_filename = os.path.basename(old_filepath)
    new_filepath = os.path.join(DST_DIRECTORY, old_filename)
    with zip_file.open(old_filepath) as f:
        buffer = f.read()
    image = load_hdr_image(buffer)
    buffer = dump_hdr_image(image)
    with open(new_filepath, "wb") as f:
        f.write(buffer)


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    with zipfile.ZipFile(SRC_FILENAME) as zip_file:
        for old_filepath in zip_file.namelist():
            if "footprint_court/diffuse/diffuse_" in old_filepath:
                compress_image(zip_file, old_filepath)
            elif "footprint_court/specular/specular_" in old_filepath:
                compress_image(zip_file, old_filepath)


if __name__ == "__main__":
    main()
