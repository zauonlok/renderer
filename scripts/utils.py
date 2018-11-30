import struct


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


def parse_element_data(element_type, component_type, element_buffer):
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


def parse_accessor_data(gltf, buffer, accessor_index):
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
        element_data = parse_element_data(
            element_type, component_type, element_buffer
        )
        accessor_data.append(element_data)

    return accessor_data


def load_gltf_mesh(gltf, buffer, mesh):
    assert len(mesh["primitives"]) == 1
    primitive = mesh["primitives"][0]
    attributes = primitive["attributes"]

    positions = parse_accessor_data(gltf, buffer, attributes["POSITION"])
    normals = parse_accessor_data(gltf, buffer, attributes["NORMAL"])
    tangents = parse_accessor_data(gltf, buffer, attributes["TANGENT"])
    texcoords = parse_accessor_data(gltf, buffer, attributes["TEXCOORD_0"])
    indices = parse_accessor_data(gltf, buffer, primitive["indices"])

    assert len(indices) % 3 == 0
    for x in [normals, tangents, texcoords]:
        assert len(x) == len(positions)

    mesh_data = {
        "positions": positions,
        "normals": normals,
        "tangents": tangents,
        "texcoords": texcoords,
        "indices": indices,
    }

    return mesh_data


def dump_mesh_data(mesh_data):
    lines = []

    for position in mesh_data["positions"]:
        p_x, p_y, p_z = position
        line = "v {:.6f} {:.6f} {:.6f}".format(p_x, p_y, p_z)
        lines.append(line)

    for texcoord in mesh_data["texcoords"]:
        t_u, t_v = texcoord
        line = "vt {:.6f} {:.6f}".format(t_u, t_v)
        lines.append(line)

    for normal in mesh_data["normals"]:
        n_x, n_y, n_z = normal
        line = "vn {:.6f} {:.6f} {:.6f}".format(n_x, n_y, n_z)
        lines.append(line)

    indices = mesh_data["indices"]
    for i in range(0, len(indices), 3):
        i_x, i_y, i_z = indices[i:(i + 3)]
        line = "f {0}/{0}/{0} {1}/{1}/{1} {2}/{2}/{2}".format(
            i_x + 1, i_y + 1, i_z + 1
        )
        lines.append(line)

    dump_data = "\n".join(lines) + "\n"
    return dump_data
