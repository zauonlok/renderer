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
    archive_name, name_list = ARCHIVE_INFO[system_name]
    os.makedirs(archive_name)

    for name in name_list:
        old_path = os.path.join("..", name)
        new_path = os.path.join(archive_name, name)
        if os.path.isdir(old_path):
            shutil.copytree(old_path, new_path)
        else:
            shutil.copy(old_path, new_path)

    archive_path = os.path.join("..", archive_name)
    shutil.make_archive(archive_path, "zip", None, archive_name)
    shutil.rmtree(archive_name)


if __name__ == "__main__":
    main()
