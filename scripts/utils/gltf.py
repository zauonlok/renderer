import functools
import math
import struct


#
# accessor parsing
#


class ElementType:

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


class ComponentType:

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
        element_data = _parse_element_data(
            element_type, component_type, element_buffer
        )
        accessor_data.append(element_data)

    return accessor_data


#
# mesh loading
#


def _load_mesh_data(gltf, buffer, mesh_index, with_tangent, with_skin):
    mesh = gltf["meshes"][mesh_index]
    primitive = mesh["primitives"][0]
    attributes = primitive["attributes"]
    accessor_parser = functools.partial(parse_accessor_data, gltf, buffer)

    positions = accessor_parser(attributes["POSITION"])
    indices = accessor_parser(primitive["indices"])
    assert len(indices) % 3 == 0

    if "TEXCOORD_0" in attributes:
        texcoords = accessor_parser(attributes["TEXCOORD_0"])
        assert len(texcoords) == len(positions)
    else:
        texcoords = [[0, 0]] * len(positions)

    if "NORMAL" in attributes:
        normals = accessor_parser(attributes["NORMAL"])
        assert len(normals) == len(positions)
    else:
        normals = [[0, 0, 1]] * len(positions)

    if with_tangent:
        tangents = accessor_parser(attributes["TANGENT"])
        assert len(tangents) == len(positions)
    else:
        tangents = []

    if with_skin:
        joints = accessor_parser(attributes["JOINTS_0"])
        weights = accessor_parser(attributes["WEIGHTS_0"])
        assert len(joints) == len(positions)
        assert len(weights) == len(positions)
    else:
        joints = []
        weights = []

    mesh_data = {
        "indices": indices,
        "positions": positions,
        "texcoords": texcoords,
        "normals": normals,
        "tangents": tangents,
        "joints": joints,
        "weights": weights,
    }
    return mesh_data


def dump_obj_data(gltf, buffer, mesh_index,
                  with_tangent=False, with_skin=False):
    mesh_data = _load_mesh_data(
        gltf, buffer, mesh_index, with_tangent, with_skin
    )

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

    for tangent in mesh_data["tangents"]:
        t_x, t_y, t_z, t_w = tangent
        line = "# ext.tangent {:.6f} {:.6f} {:.6f} {}".format(
            t_x, t_y, t_z, t_w
        )
        lines.append(line)

    for joint in mesh_data["joints"]:
        j_a, j_b, j_c, j_d = joint
        line = "# ext.joint {} {} {} {}".format(j_a, j_b, j_c, j_d)
        lines.append(line)

    for weight in mesh_data["weights"]:
        w_a, w_b, w_c, w_d = weight
        line = "# ext.weight {:.6f} {:.6f} {:.6f} {:.6f}".format(
            w_a, w_b, w_c, w_d
        )
        lines.append(line)

    obj_data = "\n".join(lines) + "\n"
    return obj_data


#
# animation loading
#


def _normalize_floats(floats):
    return [float(format(x, ".6f")) for x in floats]


def _compact_inputs(input_values, output_values):
    keyframes = []
    seen_inputs = set()
    for input_value, output_values in zip(input_values, output_values):
        if input_value not in seen_inputs:
            keyframes.append([input_value, output_values])
            seen_inputs.add(input_value)
    keyframes.sort()
    input_values, output_values = zip(*keyframes)
    return input_values, output_values


def _compact_outputs(input_values, output_values):
    keyframes = []
    num_indices = min(len(input_values), len(output_values))
    for index in range(num_indices):
        this_input = input_values[index]
        this_output = output_values[index]
        if index == 0 or index == num_indices - 1:
            keyframes.append([this_input, this_output])
        else:
            prev_value = output_values[index - 1]
            next_value = output_values[index + 1]
            if not (this_output == prev_value and this_output == next_value):
                keyframes.append([this_input, this_output])
    input_values, output_values = zip(*keyframes)
    return input_values, output_values


def _build_keyframes(input_data, output_data):
    input_values = _normalize_floats(input_data)
    output_values = [_normalize_floats(x) for x in output_data]
    input_values, output_values = _compact_inputs(input_values, output_values)
    input_values, output_values = _compact_outputs(input_values, output_values)
    if len(output_values) == 2 and output_values[0] == output_values[1]:
        return [[input_values[0], output_values[0]]]
    else:
        return list(zip(input_values, output_values))


def _load_animation(accessor_parser, animation_data, node_index, node_data):
    translations = []
    rotations = []
    scales = []
    for channel in animation_data["channels"]:
        channel_target = channel["target"]
        if channel_target["node"] == node_index:
            if channel_target["path"] != "weights":
                sampler_index = channel["sampler"]
                channel_sampler = animation_data["samplers"][sampler_index]
                input_data = accessor_parser(channel_sampler["input"])
                output_data = accessor_parser(channel_sampler["output"])
                assert len(input_data) == len(output_data)
                keyframes = _build_keyframes(input_data, output_data)
                if channel_target["path"] == "translation":
                    translations = keyframes
                elif channel_target["path"] == "rotation":
                    rotations = keyframes
                elif channel_target["path"] == "scale":
                    scales = keyframes
                else:
                    raise Exception("unknown target path")

    if not translations and "translation" in node_data:
        translations = [(0.0, node_data["translation"])]
    if not rotations and "rotation" in node_data:
        rotations = [(0.0, node_data["rotation"])]
    if not scales and "scale" in node_data:
        scales = [(0.0, node_data["scale"])]

    return translations, rotations, scales


def _dump_ani_data(joints):
    min_time = float("+inf")
    max_time = float("-inf")
    for joint in joints:
        keyframes = joint["translations"] + joint["rotations"] + joint["scales"]
        for time, _ in keyframes:
            min_time = min(min_time, time)
            max_time = max(max_time, time)

    lines = []

    line = "joint-size: {}".format(len(joints))
    lines.append(line)

    line = "time-range: [{:.6f}, {:.6f}]".format(min_time, max_time)
    lines.append(line)

    for joint in joints:
        line = "\n" + "joint {}:".format(joint["joint_index"])
        lines.append(line)

        line = "    parent-index: {}".format(joint["parent_index"])
        lines.append(line)

        line = "    inverse-bind:"
        lines.append(line)
        inverse_bind = joint["inverse_bind"]
        for i in range(4):
            line = "        {:.6f} {:.6f} {:.6f} {:.6f}".format(
                inverse_bind[0 + i], inverse_bind[4 + i],
                inverse_bind[8 + i], inverse_bind[12 + i]
            )
            lines.append(line)

        line = "    translations {}:".format(len(joint["translations"]))
        lines.append(line)
        for time, translation in joint["translations"]:
            time = format(time, ".6f")
            value = "[{:.6f}, {:.6f}, {:.6f}]".format(*translation)
            line = "        time: {}, value: {}".format(time, value)
            lines.append(line)

        line = "    rotations {}:".format(len(joint["rotations"]))
        lines.append(line)
        for time, rotation in joint["rotations"]:
            time = format(time, ".6f")
            value = "[{:.6f}, {:.6f}, {:.6f}, {:.6f}]".format(*rotation)
            line = "        time: {}, value: {}".format(time, value)
            lines.append(line)

        line = "    scales {}:".format(len(joint["scales"]))
        lines.append(line)
        for time, scale in joint["scales"]:
            time = format(time, ".6f")
            value = "[{:.6f}, {:.6f}, {:.6f}]".format(*scale)
            line = "        time: {}, value: {}".format(time, value)
            lines.append(line)

    ani_data = "\n".join(lines) + "\n"
    return ani_data


def dump_skin_ani_data(gltf, buffer, animation_index=0, skin_index=0):
    accessor_parser = functools.partial(parse_accessor_data, gltf, buffer)
    animation_data = gltf["animations"][animation_index]
    skin_data = gltf["skins"][skin_index]

    root_index = skin_data["skeleton"]
    inverse_bind_matrices = accessor_parser(skin_data["inverseBindMatrices"])
    assert len(inverse_bind_matrices) == len(skin_data["joints"])

    child_to_parent = {}
    for node_index, node_data in enumerate(gltf["nodes"]):
        for child_index in node_data.get("children", []):
            assert child_index not in child_to_parent
            child_to_parent[child_index] = node_index

    joints = []
    for joint_index, node_index in enumerate(skin_data["joints"]):
        inverse_bind = inverse_bind_matrices[joint_index]

        if node_index == root_index:
            parent_index = -1
        else:
            parent_node = child_to_parent[node_index]
            parent_index = skin_data["joints"].index(parent_node)
            assert 0 <= parent_index < joint_index

        translations, rotations, scales = _load_animation(
            accessor_parser, animation_data,
            node_index, gltf["nodes"][node_index]
        )

        joint = {
            "joint_index": joint_index,
            "parent_index": parent_index,
            "inverse_bind": inverse_bind,
            "translations": translations,
            "rotations": rotations,
            "scales": scales,
        }
        joints.append(joint)

    ani_data = _dump_ani_data(joints)
    return ani_data


IDENTITY_MATRIX = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]


def _has_transform(joint):
    if joint["translations"] or joint["rotations"] or joint["scales"]:
        return True
    elif joint["inverse_bind"] != IDENTITY_MATRIX:
        return True
    else:
        return False


def _has_meshes(joint, joints):
    if joint["meshes"]:
        return True

    for child_index in joint["children"]:
        child_joint = joints[child_index]
        if _has_meshes(child_joint, joints):
            return True

    return False


def _compact_joints(joints):
    compacted = [joints[0]]
    for joint in joints[1:]:
        parent_index = joint["parent_index"]
        parent_joint = joints[parent_index]
        if parent_joint["discarded"]:
            assert not parent_joint["meshes"]
            parent_index = parent_joint["parent_index"]
            parent_joint = joints[parent_index]
            assert not parent_joint["discarded"]
        joint["parent_index"] = parent_index

        if _has_transform(joint) and _has_meshes(joint, joints):
            compacted.append(joint)
        else:
            joint["discarded"] = True
            parent_joint["meshes"].extend(joint["meshes"])
            joint["meshes"] = []

    mesh2joint_dict = {}
    for joint_index, joint in enumerate(compacted):
        if joint_index > 0:
            joint["joint_index"] = joint_index
            parent_index = joint["parent_index"]
            parent_joint = joints[parent_index]
            joint["parent_index"] = compacted.index(parent_joint)
        for mesh_index in joint["meshes"]:
            mesh2joint_dict[mesh_index] = joint_index

    mesh2joint_list = []
    for mesh_index in range(len(mesh2joint_dict)):
        joint_index = mesh2joint_dict[mesh_index]
        mesh2joint_list.append(joint_index)

    return compacted, mesh2joint_list


def dump_node_ani_data(gltf, buffer, animation_index=0):
    accessor_parser = functools.partial(parse_accessor_data, gltf, buffer)
    animation_data = gltf["animations"][animation_index]

    child_to_parent = {}
    for node_index, node_data in enumerate(gltf["nodes"]):
        for child_index in node_data.get("children", []):
            assert child_index not in child_to_parent
            child_to_parent[child_index] = node_index

    joints = []
    for node_index, node_data in enumerate(gltf["nodes"]):
        if node_index == 0:
            parent_index = -1
        else:
            parent_index = child_to_parent[node_index]
            assert 0 <= parent_index < node_index

        if "matrix" in node_data:
            inverse_bind = node_data["matrix"]
        else:
            inverse_bind = IDENTITY_MATRIX

        translations, rotations, scales = _load_animation(
            accessor_parser, animation_data, node_index, node_data
        )

        meshes = [node_data["mesh"]] if "mesh" in node_data else []
        children = node_data.get("children", [])

        joint = {
            "joint_index": node_index,
            "parent_index": parent_index,
            "inverse_bind": inverse_bind,
            "translations": translations,
            "rotations": rotations,
            "scales": scales,

            "meshes": meshes,
            "children": children,
            "discarded": False,
        }
        joints.append(joint)

    joints, mesh2joint = _compact_joints(joints)
    ani_data = _dump_ani_data(joints)
    return ani_data, mesh2joint


#
# node loading
#


class Node:

    def __init__(self):
        self.mesh = None
        self.parent = None
        self.children = []
        self.local_transform = None
        self.world_transform = None


class Transform:

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
        length = math.sqrt(x*x + y*y + z*z + w*w)
        x, y, z, w = x/length, y/length, z/length, w/length
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

    def __eq__(self, other):
        return isinstance(other, Transform) and self.data == other.data


def load_node_data(gltf):
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
