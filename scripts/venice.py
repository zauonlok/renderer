"""Preprocess the Venice Sunset environment map

The environment map is available for download from
    https://hdrihaven.com/files/hdris/venice_sunset_8k.hdr
"""

from __future__ import print_function

from .utils import cmgen

SRC_FILENAME = "venice_sunset_8k.hdr"
DST_DIRECTORY = "../assets/venice"
DST_LOOKUP_TEX = "../assets/common/brdf_lut"


def main():
    cmgen.gen_environment_maps(SRC_FILENAME, DST_DIRECTORY)
    cmgen.enlarge_specular_maps(SRC_FILENAME, DST_DIRECTORY)
    cmgen.gen_lookup_texture(DST_LOOKUP_TEX)


if __name__ == "__main__":
    main()
