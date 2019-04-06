import math
import re


#
# standard conversions
#


def _float_to_rgbe(rv, gv, bv):
    assert isinstance(rv, float)  # red value
    assert isinstance(gv, float)  # green value
    assert isinstance(bv, float)  # blue value

    max_v = max(rv, gv, bv)
    if max_v < 1e-32:
        rm = 0  # red mantissa
        gm = 0  # green mantissa
        bm = 0  # blue mantissa
        eb = 0  # exponent biased
    else:
        max_m, ev = math.frexp(max_v)
        factor = (1.0 / max_v) * max_m * 256.0
        rm = int(rv * factor)  # red mantissa
        gm = int(gv * factor)  # green mantissa
        bm = int(bv * factor)  # blue mantissa
        eb = int(ev + 128.0)   # exponent biased

    return rm, gm, bm, eb


def _rgbe_to_float(rm, gm, bm, eb):
    assert isinstance(rm, int)  # red mantissa
    assert isinstance(gm, int)  # green mantissa
    assert isinstance(bm, int)  # blue mantissa
    assert isinstance(eb, int)  # exponent biased

    if eb == 0:
        rv = 0.0  # red value
        gv = 0.0  # green value
        bv = 0.0  # blue value
    else:
        ev = eb - 128.0
        factor = (1.0 / 256.0) * pow(2.0, ev)
        rv = rm * factor  # red value
        gv = gm * factor  # green value
        bv = bm * factor  # blue value

    return rv, gv, bv


#
# rgbe loading
#


def _load_next_line(buffer, begin):
    current = begin

    while current < len(buffer):
        if buffer[current] == ord("\n"):
            line = buffer[begin:current]
            return line.decode("ascii"), current + 1
        else:
            current += 1

    raise Exception("line not found")


def _load_hdr_header(buffer, begin):
    current = begin

    signature_line, current = _load_next_line(buffer, current)
    assert signature_line.startswith("#?")

    header_found = False
    format_found = False
    while True:
        header_line, current = _load_next_line(buffer, current)
        if header_line == "":
            header_found = True
            break
        elif header_line.startswith("FORMAT="):
            assert header_line == "FORMAT=32-bit_rle_rgbe"
            format_found = True
        elif header_line.startswith("GAMMA="):
            pass
        elif header_line.startswith("EXPOSURE="):
            pass
        elif header_line.startswith("#"):
            pass
        else:
            raise Exception("unknown header line: {}".format(header_line))
    assert header_found and format_found

    resolution_line, current = _load_next_line(buffer, current)
    match = re.match("-Y (\\d+) \\+X (\\d+)", resolution_line)
    assert match is not None
    height = int(match.group(1))
    width = int(match.group(2))
    assert width > 0 and height > 0

    return width, height, current


def _load_flat_scanline(buffer, begin, width):
    current = begin

    scanline = []
    for _ in range(width):
        rm, gm, bm, eb = buffer[current:(current + 4)]
        pixel = _rgbe_to_float(rm, gm, bm, eb)
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
                values = [buffer[current + 1]] * count
                current += 2
            else:
                count = buffer[current]
                assert count > 0 and size + count <= width
                values = buffer[(current + 1):(current + 1 + count)]
                current += 1 + count
            channels[i].extend(values)
            size += count
        assert size == width

    scanline = []
    for i in range(width):
        rm = channels[0][i]
        gm = channels[1][i]
        bm = channels[2][i]
        eb = channels[3][i]
        pixel = _rgbe_to_float(rm, gm, bm, eb)
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
            assert (byte2 << 8) | byte3 == width
            return _load_rle_scanline(buffer, begin + 4, width)


def load_hdr_image(buffer):
    buffer = bytearray(buffer)
    current = 0
    image = []

    width, height, current = _load_hdr_header(buffer, current)
    for _ in range(height):
        scanline, current = _load_hdr_scanline(buffer, current, width)
        image.append(scanline)

    assert current == len(buffer)
    return image


#
# rgbe dumping
#


def _dump_hdr_header(buffer, width, height):
    buffer.extend(b"#?RADIANCE\n")
    buffer.extend(b"FORMAT=32-bit_rle_rgbe\n")
    buffer.extend(b"\n")
    buffer.extend("-Y {} +X {}\n".format(height, width).encode("ascii"))


def _dump_flat_scanline(buffer, scanline):
    for pixel in scanline:
        rv, gv, bv = pixel
        rgbe = _float_to_rgbe(rv, gv, bv)
        buffer.extend(rgbe)


def _dump_rle_channel(buffer, channel, width):
    min_length = 4
    size = 0

    while size < width:
        run_begin = size
        run_count = 1
        while run_begin < width:
            while run_begin + run_count < width and run_count < 127:
                if channel[run_begin + run_count] == channel[run_begin]:
                    run_count += 1
                else:
                    break
            if run_count < min_length:
                run_begin += run_count
                run_count = 1
            else:
                break

        while size < run_begin:
            raw_count = min(run_begin - size, 128)
            buffer.append(raw_count)
            buffer.extend(channel[size:(size + raw_count)])
            size += raw_count

        if run_count >= min_length:
            buffer.append(run_count + 128)
            buffer.append(channel[run_begin])
            size += run_count

    assert size == width


def _dump_rle_scanline(buffer, scanline):
    width = len(scanline)
    byte0 = 2
    byte1 = 2
    byte2 = width >> 8
    byte3 = width & 0xff
    buffer.extend([byte0, byte1, byte2, byte3])

    channels = [[], [], [], []]
    for pixel in scanline:
        rv, gv, bv = pixel
        rm, gm, bm, eb = _float_to_rgbe(rv, gv, bv)
        channels[0].append(rm)
        channels[1].append(gm)
        channels[2].append(bm)
        channels[3].append(eb)

    for i in range(4):
        _dump_rle_channel(buffer, channels[i], width)


def _dump_hdr_scanline(buffer, scanline):
    width = len(scanline)
    if width < 8 or width > 0x7fff:
        _dump_flat_scanline(buffer, scanline)
    else:
        _dump_rle_scanline(buffer, scanline)


def dump_hdr_image(image):
    width = len(image[0])
    height = len(image)
    buffer = bytearray()

    _dump_hdr_header(buffer, width, height)
    for scanline in image:
        assert len(scanline) == width
        _dump_hdr_scanline(buffer, scanline)

    return buffer
