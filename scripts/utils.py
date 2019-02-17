import functools
import math
import struct


#
# gltf mesh loading
#


class ElementType(object):

    SCALAR = "SCALAR"
    VEC2 = "VEC2"
    VEC3 = "VEC3"
    VEC4 = "VEC4"
    MAT2 = "MAT2"
    MAT3 = "MAT3"
    MAT4 = "MAT4"


NUM_COMPONENTS = {
    ElementType.SCALAR: 1,
    ElementType.VEC2: 2,
    ElementType.VEC3: 3,
    ElementType.VEC4: 4,
    ElementType.MAT2: 4,
    ElementType.MAT3: 9,
    ElementType.MAT4: 16,
}


class ComponentType(object):

    BYTE = 5120
    UNSIGNED_BYTE = 5121
    SHORT = 5122
    UNSIGNED_SHORT = 5123
    UNSIGNED_INT = 5125
    FLOAT = 5126


COMPONENT_SIZE = {
    ComponentType.BYTE: 1,
    ComponentType.UNSIGNED_BYTE: 1,
    ComponentType.SHORT: 2,
    ComponentType.UNSIGNED_SHORT: 2,
    ComponentType.UNSIGNED_INT: 4,
    ComponentType.FLOAT: 4,
}


COMPONENT_PARSERS = {
    ComponentType.BYTE: lambda x: struct.unpack("<b", x)[0],
    ComponentType.UNSIGNED_BYTE: lambda x: struct.unpack("<B", x)[0],
    ComponentType.SHORT: lambda x: struct.unpack("<h", x)[0],
    ComponentType.UNSIGNED_SHORT: lambda x: struct.unpack("<H", x)[0],
    ComponentType.UNSIGNED_INT: lambda x: struct.unpack("<I", x)[0],
    ComponentType.FLOAT: lambda x: struct.unpack("<f", x)[0],
}


def _parse_element_data(element_type, component_type, element_buffer):
    num_components = NUM_COMPONENTS[element_type]
    component_size = COMPONENT_SIZE[component_type]
    component_parser = COMPONENT_PARSERS[component_type]
    if num_components == 1:
        component_data = component_parser(element_buffer)
        return component_data
    else:
        element_data = []
        for i in range(num_components):
            component_begin = i * component_size
            component_end = component_begin + component_size
            component_buffer = element_buffer[component_begin:component_end]
            component_data = component_parser(component_buffer)
            element_data.append(component_data)
        return element_data


def _parse_accessor_data(gltf, buffer, accessor_index):
    accessor = gltf["accessors"][accessor_index]
    view_index = accessor["bufferView"]
    view = gltf["bufferViews"][view_index]
    assert view["buffer"] == 0

    accessor_offset = accessor.get("byteOffset", 0)
    view_offset = view.get("byteOffset", 0)
    buffer_offset = accessor_offset + view_offset

    element_type = accessor["type"]
    component_type = accessor["componentType"]

    num_components = NUM_COMPONENTS[element_type]
    component_size = COMPONENT_SIZE[component_type]
    element_size = num_components * component_size

    view_stride = view.get("byteStride", 0)
    element_stride = max(element_size, view_stride)

    element_count = accessor["count"]
    view_length = view["byteLength"]
    required_size = element_stride * (element_count - 1) + element_size
    assert accessor_offset + required_size <= view_length

    accessor_data = []
    for i in range(element_count):
        element_begin = buffer_offset + i * element_stride
        element_end = element_begin + element_size
        element_buffer = buffer[element_begin:element_end]
        element_data = _parse_element_data(
            element_type, component_type, element_buffer
        )
        accessor_data.append(element_data)

    return accessor_data


def load_gltf_mesh(gltf, buffer, mesh):
    assert len(mesh["primitives"]) == 1
    primitive = mesh["primitives"][0]
    attributes = primitive["attributes"]
    accessor_parser = functools.partial(_parse_accessor_data, gltf, buffer)

    indices = accessor_parser(primitive["indices"])
    positions = accessor_parser(attributes["POSITION"])

    if "NORMAL" in attributes:
        normals = accessor_parser(attributes["NORMAL"])
    else:
        normals = None

    if "TANGENT" in attributes:
        tangents = accessor_parser(attributes["TANGENT"])
    else:
        tangents = None

    if "TEXCOORD_0" in attributes:
        texcoords = accessor_parser(attributes["TEXCOORD_0"])
    else:
        texcoords = None

    mesh_data = {
        "indices": indices,
        "positions": positions,
        "normals": normals,
        "tangents": tangents,
        "texcoords": texcoords,
    }

    return mesh_data


def _dump_obj_data(positions, texcoords, normals, indices):
    lines = []

    for position in positions:
        p_x, p_y, p_z = position
        line = "v {:.6f} {:.6f} {:.6f}".format(p_x, p_y, p_z)
        lines.append(line)

    for texcoord in texcoords:
        t_u, t_v = texcoord
        line = "vt {:.6f} {:.6f}".format(t_u, t_v)
        lines.append(line)

    for normal in normals:
        n_x, n_y, n_z = normal
        line = "vn {:.6f} {:.6f} {:.6f}".format(n_x, n_y, n_z)
        lines.append(line)

    for i in range(0, len(indices), 3):
        i_x, i_y, i_z = indices[i:(i + 3)]
        line = "f {0}/{0}/{0} {1}/{1}/{1} {2}/{2}/{2}".format(
            i_x + 1, i_y + 1, i_z + 1
        )
        lines.append(line)

    obj_data = "\n".join(lines) + "\n"
    return obj_data


def _dump_tan_data(tangents, indices):
    lines = []

    for i in indices:
        t_x, t_y, t_z, t_w = tangents[i]
        line = "tan {:.6f} {:.6f} {:.6f} {:.6f}".format(t_x, t_y, t_z, t_w)
        lines.append(line)

    tan_data = "\n".join(lines) + "\n"
    return tan_data


def dump_mesh_data(mesh_data):
    positions = mesh_data["positions"]
    texcoords = mesh_data["texcoords"]
    normals = mesh_data["normals"]
    tangents = mesh_data["tangents"]
    indices = mesh_data["indices"]

    assert len(indices) % 3 == 0

    if texcoords is None:
        texcoords = [[0, 0]] * len(positions)
    else:
        assert len(texcoords) == len(positions)

    if normals is None:
        normals = [[0, 0, 1]] * len(positions)
    else:
        assert len(normals) == len(positions)

    if tangents is None:
        tangents = [[1, 0, 0, 1]] * len(positions)
    else:
        assert len(tangents) == len(positions)

    obj_data = _dump_obj_data(positions, texcoords, normals, indices)
    tan_data = _dump_tan_data(tangents, indices)

    return obj_data, tan_data


#
# gltf node loading
#


class Node(object):

    def __init__(self):
        self.mesh = None
        self.parent = None
        self.children = []
        self.local_transform = None
        self.world_transform = None


class Transform(object):

    def __init__(self, data):
        assert len(data) == 4 and all(len(row) == 4 for row in data)
        self.data = data

    @staticmethod
    def from_translation(translation):
        x, y, z = translation
        data = [
            [1, 0, 0, x],
            [0, 1, 0, y],
            [0, 0, 1, z],
            [0, 0, 0, 1],
        ]
        return Transform(data)

    @staticmethod
    def from_rotation(rotation):
        x, y, z, w = rotation
        sqrt_norm = math.sqrt(x*x + y*y + z*z + w*w)
        x, y, z, w = x/sqrt_norm, y/sqrt_norm, z/sqrt_norm, w/sqrt_norm
        data = [
            [1 - 2*y*y - 2*z*z, 2*x*y - 2*w*z,     2*x*z + 2*y*w,     0],
            [2*x*y + 2*w*z,     1 - 2*x*x - 2*z*z, 2*y*z - 2*w*x,     0],
            [2*x*z - 2*w*y,     2*y*z + 2*w*x,     1 - 2*x*x - 2*y*y, 0],
            [0,                 0,                 0,                 1],
        ]
        return Transform(data)

    @staticmethod
    def from_scale(scale):
        x, y, z = scale
        data = [
            [x, 0, 0, 0],
            [0, y, 0, 0],
            [0, 0, z, 0],
            [0, 0, 0, 1],
        ]
        return Transform(data)

    @staticmethod
    def from_matrix(matrix):
        data = [
            [matrix[0], matrix[4], matrix[8],  matrix[12]],
            [matrix[1], matrix[5], matrix[9],  matrix[13]],
            [matrix[2], matrix[6], matrix[10], matrix[14]],
            [matrix[3], matrix[7], matrix[11], matrix[15]],
        ]
        return Transform(data)

    def __mul__(self, other):
        data = [
            [0, 0, 0, 0],
            [0, 0, 0, 0],
            [0, 0, 0, 0],
            [0, 0, 0, 0],
        ]
        for i in range(4):
            for j in range(4):
                for k in range(4):
                    data[i][j] += self.data[i][k] * other.data[k][j]
        return Transform(data)


def load_gltf_nodes(gltf):
    num_nodes = len(gltf["nodes"])
    nodes = [Node() for _ in range(num_nodes)]

    for i in range(num_nodes):
        this_node = nodes[i]
        node_data = gltf["nodes"][i]

        if "mesh" in node_data:
            this_node.mesh = node_data["mesh"]

        for j in node_data.get("children", []):
            child = nodes[j]
            assert child.parent is None
            child.parent = this_node
            this_node.children.append(child)

        if "matrix" in node_data:
            assert "scale" not in node_data
            assert "rotation" not in node_data
            assert "translation" not in node_data
            local_transform = Transform.from_matrix(node_data["matrix"])
        else:
            scale = node_data.get("scale", [1, 1, 1])
            rotation = node_data.get("rotation", [0, 0, 0, 1])
            translation = node_data.get("translation", [0, 0, 0])
            scale = Transform.from_scale(scale)
            rotation = Transform.from_rotation(rotation)
            translation = Transform.from_translation(translation)
            local_transform = translation * rotation * scale
        this_node.local_transform = local_transform

    for node in nodes:
        transform = node.local_transform
        parent = node.parent
        while parent:
            transform = parent.local_transform * transform
            parent = parent.parent
        node.world_transform = transform

    return nodes


def load_gltf_transforms(gltf):
    nodes = load_gltf_nodes(gltf)
    nodes = [node for node in nodes if node.mesh is not None]

    transforms = []
    for node in nodes:
        found = False
        for transform in transforms:
            if transform.data == node.world_transform.data:
                found = True
                break
        if not found:
            transforms.append(node.world_transform)

    return transforms
