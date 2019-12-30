"""Create text image files

The Python Imaging Library is required
    pip install pillow
"""

import os

from PIL import Image, ImageDraw, ImageFont

DST_DIRECTORY = "../assets/common"

FONT_NAME = "arial.ttf"
FONT_SIZE = 14

PADDING_TOP = 6
PADDING_BOTTOM = 6
PADDING_LEFT = 8
PADDING_RIGHT = 8


def gen_text_image(text):
    font = ImageFont.truetype(FONT_NAME, FONT_SIZE)
    ascent, descent = font.getmetrics()
    height = PADDING_TOP + ascent + descent + PADDING_BOTTOM
    width = PADDING_LEFT + font.getsize(text)[0] + PADDING_RIGHT

    image = Image.new("L", (width, height))
    draw = ImageDraw.Draw(image)
    draw.text((PADDING_LEFT, PADDING_TOP), text, (127,), font)

    name = text.lower() + ".tga"
    path = os.path.join(DST_DIRECTORY, name)
    image.save(path, rle=True)


def main():
    texts = ["Diffuse", "Specular", "Roughness", "Occlusion", "Normal"]
    for text in texts:
        gen_text_image(text)


if __name__ == "__main__":
    main()
