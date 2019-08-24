"""Create text image files

The Python Imaging Library is required
    pip install pillow
"""

import os

from PIL import Image, ImageDraw, ImageFont

DST_DIRECTORY = "../assets/common"


def get_text_sizes(texts):
    image = Image.new("L", (128, 128))
    draw = ImageDraw.Draw(image)
    font = ImageFont.truetype("arial.ttf", 14)
    sizes = [draw.textsize(text, font) for text in texts]
    sizes = [(x, 18) for x, _ in sizes]
    return sizes


def gen_text_image(text, size):
    image = Image.new("L", size)
    draw = ImageDraw.Draw(image)
    font = ImageFont.truetype("arial.ttf", 14)
    draw.text((0, 0), text, (127,), font)
    name = text.lower() + ".tga"
    path = os.path.join(DST_DIRECTORY, name)
    image.save(path, rle=True)


def main():
    texts = ["Diffuse", "Specular", "Roughness", "Occlusion", "Normal"]
    sizes = get_text_sizes(texts)
    for text, size in zip(texts, sizes):
        gen_text_image(text, size)


if __name__ == "__main__":
    main()
