import math
import re


def _floats_to_rgbe(floats):
    rv = floats[0]  # red value
    gv = floats[1]  # green value
    bv = floats[2]  # blue value
    assert all(isinstance(x, float) for x in [rv, gv, bv])

    max_v = max(rv, gv, bv)
    if max_v < 1e-32:
        rm = 0  # red mantissa
        gm = 0  # green mantissa
        bm = 0  # blue mantissa
        ex = 0  # exponent biased
    else:
        max_m, ev = math.frexp(max_v)
        factor =  (1.0 / max_v) * max_m * 256.0
        rm = int(rv * factor)  # red mantissa
        gm = int(gv * factor)  # green mantissa
        bm = int(bv * factor)  # blue mantissa
        ex = int(ev + 128.0)   # exponent biased

    return rm, gm, bm, ex


def _rgbe_to_floats(rgbe):
    rm = rgbe[0]  # red mantissa
    gm = rgbe[1]  # green mantissa
    bm = rgbe[2]  # blue mantissa
    ex = rgbe[3]  # exponent biased
    assert all(isinstance(x, int) for x in [rm, gm, bm, ex])

    if ex == 0:
        rv = 0.0  # red value
        gv = 0.0  # green value
        bv = 0.0  # blue value
    else:
        ev = ex - 128.0
        factor = (1.0 / 256.0) * pow(2.0, ev)
        rv = rm * factor  # red value
        gv = gm * factor  # green value
        bv = bm * factor  # blue value

    return rv, gv, bv


def _load_next_line(buffer, begin):
    current = begin

    while current < len(buffer):
        if buffer[current] == b"\n"[0]:
            line = buffer[begin:current]
            return line, current + 1
        else:
            current += 1

    raise Exception("line not found")


def _load_hdr_header(buffer, begin):
    current = begin

    signature_line, current = _load_next_line(buffer, current)
    assert signature_line.startswith(b"#?")

    header_found = False
    format_found = False
    while True:
        header_line, current = _load_next_line(buffer, current)
        if header_line == b"":
            header_found = True
            break
        elif header_line.startswith(b"FORMAT="):
            assert header_line == b"FORMAT=32-bit_rle_rgbe"
            format_found = True
        elif header_line.startswith(b"GAMMA="):
            pass
        elif header_line.startswith(b"EXPOSURE="):
            pass
        elif header_line.startswith(b"#"):
            pass
        else:
            raise Exception("unknown header line: {}".format(header_line))
    assert header_found and format_found

    resolution_line, current = _load_next_line(buffer, current)
    match = re.match(b"-Y (\\d+) \\+X (\\d+)", resolution_line)
    assert match is not None
    height = int(match.group(1))
    width = int(match.group(2))
    assert width > 0 and height > 0

    return width, height, current


def _load_flat_scanline(buffer, begin, width):
    current = begin

    scanline = []
    for _ in range(width):
        rgbe = buffer[current:(current + 4)]
        pixel = _rgbe_to_floats(rgbe)
        scanline.append(pixel)
        current += 4

    return scanline, current


def _load_rle_scanline(buffer, begin, width):
    current = begin

    channels = [[], [], [], []]
    for i in range(4):
        size = 0
        while size < width:
            if buffer[current] > 128:
                count = buffer[current] - 128
                assert count > 0 and size + count <= width
                current += 1
                for _ in range(count):
                    channels[i].append(buffer[current])
                current += 1
                size += count
            else:
                count = buffer[current]
                assert count > 0 and size + count <= width
                current += 1
                for _ in range(count):
                    channels[i].append(buffer[current])
                    current += 1
                size += count
        assert size == width

    scanline = []
    for i in range(width):
        rm = channels[0][i]
        gm = channels[1][i]
        bm = channels[2][i]
        ex = channels[3][i]
        pixel = _rgbe_to_floats([rm, gm, bm, ex])
        scanline.append(pixel)

    return scanline, current


def _load_hdr_scanline(buffer, begin, width):
    if width < 8 or width > 0x7fff:
        return _load_flat_scanline(buffer, begin, width)
    else:
        byte0, byte1, byte2, byte3 = buffer[begin:(begin + 4)]
        if byte0 != 2 or byte1 != 2 or byte2 & 0x80:
            return _load_flat_scanline(buffer, begin, width)
        else:
            assert byte2 * 256 + byte3 == width
            return _load_rle_scanline(buffer, begin + 4, width)


def load_hdr_image(buffer):
    current = 0

    width, height, current = _load_hdr_header(buffer, current)
    buffer = bytearray(buffer)

    image = []
    for _ in range(height):
        scanline, current = _load_hdr_scanline(buffer, current, width)
        image.append(scanline)

    assert current == len(buffer)
    return image
