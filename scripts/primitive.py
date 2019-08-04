"""Create simple geometric primitives"""

import math
import os

DST_DIRECTORY = "../assets/common"


def dump_obj_data(filename, positions, texcoords, normals, indices):
    lines = []

    for position in positions:
        line = "v {:.6f} {:.6f} {:.6f}".format(*position)
        lines.append(line)

    for texcoord in texcoords:
        line = "vt {:.6f} {:.6f}".format(*texcoord)
        lines.append(line)

    for normal in normals:
        line = "vn {:.6f} {:.6f} {:.6f}".format(*normal)
        lines.append(line)

    for i, j, k in indices:
        line = "f {0}/{0}/{0} {1}/{1}/{1} {2}/{2}/{2}".format(
            i + 1, j + 1, k + 1
        )
        lines.append(line)

    obj_data = "\n".join(lines) + "\n"
    filepath = os.path.join(DST_DIRECTORY, filename)
    with open(filepath, "w") as f:
        f.write(obj_data)


def create_sphere(filename, radius, num_sectors, num_stacks):
    positions = []
    texcoords = []
    normals = []
    indices = []

    sector_step = 2 * math.pi / num_sectors
    stack_step = math.pi / num_stacks

    for i in range(num_stacks + 1):
        for j in range(num_sectors + 1):
            stack_angle = math.pi / 2 - i * stack_step      # [pi/2, -pi/2]
            sector_angle = j * sector_step                  # [0, 2*pi]
            nx = math.cos(stack_angle) * math.cos(sector_angle)
            ny = math.cos(stack_angle) * math.sin(sector_angle)
            nz = math.sin(stack_angle)

            position = [nx * radius, ny * radius, nz * radius]
            texcoord = [float(j) / num_sectors, float(i) / num_stacks]
            normal = [nx, ny, nz]

            positions.append(position)
            texcoords.append(texcoord)
            normals.append(normal)

    for i in range(num_stacks):
        curr_stack = i * (num_sectors + 1)
        next_stack = (i + 1) * (num_sectors + 1)
        for j in range(num_sectors):
            if i != 0:
                indices.append([curr_stack, next_stack, curr_stack + 1])
            if i != num_stacks - 1:
                indices.append([curr_stack + 1, next_stack, next_stack + 1])
            curr_stack += 1
            next_stack += 1

    dump_obj_data(filename, positions, texcoords, normals, indices)


def main():
    create_sphere("sphere.obj", 0.5, 50, 25)


if __name__ == "__main__":
    main()
