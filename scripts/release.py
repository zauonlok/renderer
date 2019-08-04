"""Create an archive for a release"""

from __future__ import print_function

import os
import platform
import shutil

ARCHIVE_INFO = {
    "Windows": [
        "renderer-win32", ["assets", "LICENSE", "README.md", "Viewer.exe"]
    ],
    "Darwin": [
        "renderer-macos", ["assets", "LICENSE", "README.md", "Viewer"]
    ],
    "Linux": [
        "renderer-linux", ["assets", "LICENSE", "README.md", "Viewer"]
    ],
}


def main():
    system_name = platform.system()
    archive_name, filenames = ARCHIVE_INFO[system_name]
    os.makedirs(archive_name)

    for filename in filenames:
        old_filepath = os.path.join("..", filename)
        new_filepath = os.path.join(archive_name, filename)
        if os.path.isdir(old_filepath):
            shutil.copytree(old_filepath, new_filepath)
        else:
            shutil.copy(old_filepath, new_filepath)

    archive_path = os.path.join("..", archive_name)
    shutil.make_archive(archive_path, "zip", None, archive_name)
    shutil.rmtree(archive_name)


if __name__ == "__main__":
    main()
