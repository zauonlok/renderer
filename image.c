#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "image.h"
#include "error.h"

#pragma pack(push, 1)
struct tga_header {
    uint8_t idlength;
    uint8_t colormaptype;
    uint8_t imagetype;
    uint16_t colormaporigin;
    uint16_t colormaplength;
    uint8_t colormapdepth;
    uint16_t xorigin;
    uint16_t yorigin;
    uint16_t width;
    uint16_t height;
    uint8_t pixeldepth;
    uint8_t descriptor;
};
#pragma pack(pop)

static const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (dot == NULL || dot == filename) {
        return "";
    } else {
        return dot + 1;
    }
}

static int read_byte(FILE *file) {
    int byte = fgetc(file);
    if (byte == EOF) {
        FATAL("fgetc");
    }
    return byte;
}

static void load_tga_rle(FILE *file, unsigned char *buffer,
                         int buffer_size, int channels) {
    unsigned char *pixel = (unsigned char*)malloc(channels);
    int buffer_count = 0;
    int i, j;
    do {
        int header = read_byte(file);
        if (header < 128) {     /* raw packet */
            int pixel_count = header + 1;
            if (buffer_count + pixel_count * channels > buffer_size) {
                FATAL("tga: raw packet too large");
            }
            for (i = 0; i < pixel_count; i++) {
                for (j = 0; j < channels; j++) {
                    buffer[buffer_count] = read_byte(file);
                    buffer_count += 1;
                }
            }
        } else {                /* run-length packet */
            int pixel_count = header - 128 + 1;
            if (buffer_count + pixel_count * channels > buffer_size) {
                FATAL("tga: run-length packet too large");
            }
            for (j = 0; j < channels; j++) {
                pixel[j] = read_byte(file);
            }
            for (i = 0; i < pixel_count; i++) {
                for (j = 0; j < channels; j++) {
                    buffer[buffer_count] = pixel[j];
                    buffer_count += 1;
                }
            }
        }
    } while (buffer_count < buffer_size);
    free(pixel);
}

static image_t *load_tga(const char *filename) {
    FILE *file;
    struct tga_header header;
    int header_size = sizeof(struct tga_header);
    int width, height;
    int pixeldepth, channels;
    int image_type;
    int buffer_size;
    unsigned char *buffer;
    image_t *image;
    int count;

    file = fopen(filename, "rb");
    if (file == NULL) {
        FATAL("fopen");
    }
    count = fread(&header, 1, header_size, file);
    if (count != header_size) {
        FATAL("fread");
    }
    width = header.width;
    height = header.height;
    if (width <= 0 || height <= 0) {
        FATAL("tga: bad width/height value");
    }
    pixeldepth = header.pixeldepth;
    channels = pixeldepth / 8;
    if (pixeldepth != 8 && pixeldepth != 24 && pixeldepth != 32) {
        FATAL("tga: unsupported color depth");
    }
    buffer_size = width * height * channels;
    buffer = (unsigned char *)malloc(buffer_size);
    image_type = header.imagetype;
    if (image_type == 2 || image_type == 3) {           /* uncompressed */
        count = fread(buffer, 1, buffer_size, file);
        if (count != buffer_size) {
            FATAL("fread");
        }
    } else if (image_type == 10 || image_type == 11) {  /* run-length encoded */
        load_tga_rle(file, buffer, buffer_size, channels);
    } else {
        FATAL("tga: unsupported image type");
    }
    fclose(file);

    image = (image_t*)malloc(sizeof(image_t));
    image->width    = width;
    image->height   = height;
    image->channels = channels;
    image->pitch    = width * channels;
    image->buffer   = buffer;

    if (!(header.descriptor & 0x20)) {
        image_flip_v(image);
    }
    if (header.descriptor & 0x10) {
        image_flip_h(image);
    }
    return image;
}

image_t *image_load(const char *filename) {
    const char *ext = get_filename_ext(filename);
    if (strcmp(ext, "tga") == 0) {
        return load_tga(filename);
    } else {
        FATAL("image: unsupported format");
        return NULL;
    }
}

static void save_tga(image_t *image, const char *filename) {
    FILE *file;
    struct tga_header header;
    int header_size = sizeof(struct tga_header);
    int buffer_size;
    int count;

    file = fopen(filename, "wb");
    if (file == NULL) {
        FATAL("fopen");
    }
    memset(&header, 0, header_size);
    header.width      = image->width;
    header.height     = image->height;
    header.pixeldepth = image->channels * 8;
    header.imagetype  = (image->channels == 1) ? 3 : 2;
    header.descriptor = 0x20;   /* top-left origin */
    count = fwrite(&header, 1, header_size, file);
    if (count != header_size) {
        FATAL("fwrite");
    }
    buffer_size = image->width * image->height * image->channels;
    count = fwrite(image->buffer, 1, buffer_size, file);
    if (count != buffer_size) {
        FATAL("fwrite");
    }
    fclose(file);
}

void image_save(image_t *image, const char *filename) {
    const char *ext = get_filename_ext(filename);
    if (strcmp(ext, "tga") == 0) {
        save_tga(image, filename);
    } else {
        FATAL("image: unsupported format");
    }
}

void image_free(image_t *image) {
    free(image->buffer);
    free(image);
}

static void swap_byte(unsigned char *a, unsigned char *b) {
    unsigned char t = *a;
    *a = *b;
    *b = t;
}

void image_flip_h(image_t *image) {
    unsigned char *buffer = image->buffer;
    int half_width = image->width / 2;
    int i, j, k;
    for (i = 0; i < image->height; i++) {
        for (j = 0; j < half_width; j++) {
            int index1 = i * image->pitch + j * image->channels;
            int j2 = image->width - j - 1;
            int index2 = i * image->pitch + j2 * image->channels;
            for (k = 0; k < image->channels; k++) {
                swap_byte(&buffer[index1 + k], &buffer[index2 + k]);
            }
        }
    }
}

void image_flip_v(image_t *image) {
    unsigned char *buffer = image->buffer;
    int half_height = image->height / 2;
    int i, j, k;
    for (i = 0; i < half_height; i++) {
        for (j = 0; j < image->width; j++) {
            int index1 = i * image->pitch + j * image->channels;
            int i2 = image->height - i - 1;
            int index2 = i2 * image->pitch + j * image->channels;
            for (k = 0; k < image->channels; k++) {
                swap_byte(&buffer[index1 + k], &buffer[index2 + k]);
            }
        }
    }
}
