#include "image.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* common helper functions */

static int get_buffer_size(image_t *image) {
    return image->width * image->height * image->channels;
}

static unsigned char *get_pixel_ptr(image_t *image, int row, int col) {
    int index = row * image->width * image->channels + col * image->channels;
    return &(image->buffer[index]);
}

/* image creating/releasing */

image_t *image_create(int width, int height, int channels) {
    int buffer_size = width * height * channels;
    image_t *image;

    assert(width > 0 && height > 0 && channels >= 1 && channels <= 4);

    image = (image_t*)malloc(sizeof(image_t));
    image->width    = width;
    image->height   = height;
    image->channels = channels;
    image->buffer   = (unsigned char*)malloc(buffer_size);
    memset(image->buffer, 0, buffer_size);
    return image;
}

void image_release(image_t *image) {
    free(image->buffer);
    free(image);
}

image_t *image_clone(image_t *image) {
    int buffer_size = get_buffer_size(image);
    unsigned char *buffer = (unsigned char*)malloc(buffer_size);
    image_t *clone = (image_t*)malloc(sizeof(image_t));
    *clone = *image;  /* shallow copy */
    clone->buffer = buffer;
    memcpy(clone->buffer, image->buffer, buffer_size);
    return clone;
}

/* image input/output */

static image_t *load_tga(const char *filename);
static void save_tga(image_t *image, const char *filename);

static const char *extract_ext(const char *filename) {
    const char *dot_pos = strrchr(filename, '.');
    return (dot_pos == NULL) ? "" : dot_pos + 1;
}

image_t *image_load(const char *filename) {
    const char *ext = extract_ext(filename);
    if (strcmp(ext, "tga") == 0) {
        return load_tga(filename);
    } else {
        assert(0);
        return NULL;
    }
}

void image_save(image_t *image, const char *filename) {
    const char *ext = extract_ext(filename);
    if (strcmp(ext, "tga") == 0) {
        save_tga(image, filename);
    } else {
        assert(0);
    }
}

static unsigned char read_byte(FILE *file) {
    int byte = fgetc(file);
    assert(byte != EOF);
    return (unsigned char)byte;
}

static void read_bytes(FILE *file, void *buffer, int size) {
    int count = fread(buffer, 1, size, file);
    assert(count == size);
}

static void write_bytes(FILE *file, void *buffer, int size) {
    int count = fwrite(buffer, 1, size, file);
    assert(count == size);
}

static void load_tga_rle(FILE *file, image_t *image) {
    unsigned char pixel[4];
    unsigned char *buffer = image->buffer;
    int channels = image->channels;
    int buffer_size = get_buffer_size(image);
    int buffer_count = 0;
    while (buffer_count < buffer_size) {
        unsigned char header = read_byte(file);
        int is_rle_packet = header & 0x80;
        int pixel_count = (header & 0x7F) + 1;
        int expected_size = buffer_count + pixel_count * channels;
        assert(expected_size <= buffer_size);
        if (is_rle_packet) {  /* run-length packet */
            int i, j;
            for (j = 0; j < channels; j++) {
                pixel[j] = read_byte(file);
            }
            for (i = 0; i < pixel_count; i++) {
                for (j = 0; j < channels; j++) {
                    buffer[buffer_count++] = pixel[j];
                }
            }
        } else {              /* raw packet */
            int i, j;
            for (i = 0; i < pixel_count; i++) {
                for (j = 0; j < channels; j++) {
                    buffer[buffer_count++] = read_byte(file);
                }
            }
        }
    }
}

#define TGA_HEADER_SIZE 18

static image_t *load_tga(const char *filename) {
    unsigned char header[TGA_HEADER_SIZE];
    int width, height, depth, channels;
    int idlength, imgtype, imgdesc;
    image_t *image;
    FILE *file;

    file = fopen(filename, "rb");
    assert(file != NULL);
    read_bytes(file, header, TGA_HEADER_SIZE);

    width = header[12] + (header[13] << 8);
    height = header[14] + (header[15] << 8);
    assert(width > 0 && height > 0);
    depth = header[16];
    assert(depth == 8 || depth == 24 || depth == 32);
    channels = depth / 8;
    image = image_create(width, height, channels);

    idlength = header[0];
    assert(idlength == 0);
    imgtype = header[2];
    if (imgtype == 2 || imgtype == 3) {           /* uncompressed */
        read_bytes(file, image->buffer, get_buffer_size(image));
    } else if (imgtype == 10 || imgtype == 11) {  /* run-length encoded */
        load_tga_rle(file, image);
    } else {
        assert(0);
    }
    fclose(file);

    imgdesc = header[17];
    if (imgdesc & 0x20) {
        image_flip_v(image);
    }
    if (imgdesc & 0x10) {
        image_flip_h(image);
    }
    return image;
}

static void save_tga(image_t *image, const char *filename) {
    unsigned char header[TGA_HEADER_SIZE];
    FILE *file;

    file = fopen(filename, "wb");
    assert(file != NULL);

    memset(header, 0, TGA_HEADER_SIZE);
    header[2]  = (image->channels == 1) ? 3 : 2;  /* image type */
    header[12] = image->width & 0xFF;             /* width, lsb */
    header[13] = (image->width >> 8) & 0xFF;      /* width, msb */
    header[14] = image->height & 0xFF;            /* height, lsb */
    header[15] = (image->height >> 8) & 0xFF;     /* height, msb */
    header[16] = (image->channels * 8) & 0xFF;    /* image depth */
    write_bytes(file, header, TGA_HEADER_SIZE);

    write_bytes(file, image->buffer, get_buffer_size(image));
    fclose(file);
}

/* color getting/setting */

color_t image_get_color(image_t *image, int row, int col) {
    int channels = image->channels;
    color_t color = {0, 0, 0, 255};
    unsigned char *pixel;

    assert(row >= 0 && row < image->height && col >= 0 && col < image->width);

    pixel = get_pixel_ptr(image, row, col);
    if (channels == 1) {
        color.b = color.g = color.r = pixel[0];
    } else if (channels == 2) {
        color.b = color.g = color.r = pixel[0];
        color.a = pixel[1];
    } else if (channels == 3) {
        color.b = pixel[0];
        color.g = pixel[1];
        color.r = pixel[2];
    } else if (channels == 4) {
        color.b = pixel[0];
        color.g = pixel[1];
        color.r = pixel[2];
        color.a = pixel[3];
    } else {
        assert(0);
    }

    return color;
}

static unsigned char color2gray(color_t color) {
    int gray = ((int)color.b + (int)color.g + (int)color.r) / 3;
    return (unsigned char)gray;
}

void image_set_color(image_t *image, int row, int col, color_t color) {
    int channels = image->channels;
    unsigned char *pixel;

    assert(row >= 0 && row < image->height && col >= 0 && col < image->width);

    pixel = get_pixel_ptr(image, row, col);
    if (channels == 1) {
        unsigned char gray = color2gray(color);
        pixel[0] = gray;
    } else if (channels == 2) {
        unsigned char gray = color2gray(color);
        pixel[0] = gray;
        pixel[1] = color.a;
    } else if (channels == 3) {
        pixel[0] = color.b;
        pixel[1] = color.g;
        pixel[2] = color.r;
    } else if (channels == 4) {
        pixel[0] = color.b;
        pixel[1] = color.g;
        pixel[2] = color.r;
        pixel[3] = color.a;
    } else {
        assert(0);
    }
}

/* image processing */

static void swap_bytes(unsigned char *a, unsigned char *b) {
    unsigned char t = *a;
    *a = *b;
    *b = t;
}

void image_flip_h(image_t *image) {
    int half_width = image->width / 2;
    int r, c, k;
    for (r = 0; r < image->height; r++) {
        for (c = 0; c < half_width; c++) {
            int c2 = image->width - c - 1;
            unsigned char *pixel1 = get_pixel_ptr(image, r, c);
            unsigned char *pixel2 = get_pixel_ptr(image, r, c2);
            for (k = 0; k < image->channels; k++) {
                swap_bytes(&pixel1[k], &pixel2[k]);
            }
        }
    }
}

void image_flip_v(image_t *image) {
    int half_height = image->height / 2;
    int r, c, k;
    for (r = 0; r < half_height; r++) {
        for (c = 0; c < image->width; c++) {
            int r2 = image->height - r - 1;
            unsigned char *pixel1 = get_pixel_ptr(image, r, c);
            unsigned char *pixel2 = get_pixel_ptr(image, r2, c);
            for (k = 0; k < image->channels; k++) {
                swap_bytes(&pixel1[k], &pixel2[k]);
            }
        }
    }
}

static double linear_interp(double v0, double v1, double d) {
    return v0 + (v1 - v0) * d;
}

static double bilinear_interp(double v00, double v01, double v10, double v11,
                              double drow, double dcol) {
    double v0 = linear_interp(v00, v01, dcol);  /* row 0 */
    double v1 = linear_interp(v10, v11, dcol);  /* row 1 */
    return linear_interp(v0, v1, drow);
}

static int bound_index(int index, int length) {
    return (index > length - 1) ? length - 1 : index;
}

void image_resize(image_t *image, int width, int height) {
    int channels = image->channels;
    image_t src = *image;  /* shallow copy */
    image_t *dst = image;  /* simple alias */
    double scale_r, scale_c;
    int r, c, k;

    assert(width > 0 && height > 0);

    dst->width  = width;
    dst->height = height;
    dst->buffer = (unsigned char*)malloc(width * height * channels);

    scale_r = (double)src.height / dst->height;
    scale_c = (double)src.width / dst->width;

    for (r = 0; r < dst->height; r++) {
        for (c = 0; c < dst->width; c++) {
            double mapped_r = r * scale_r;
            double mapped_c = c * scale_c;
            int src_r = (int)mapped_r;
            int src_c = (int)mapped_c;
            double drow = mapped_r - src_r;
            double dcol = mapped_c - src_c;
            int src_r_p1 = bound_index(src_r + 1, src.height);
            int src_c_p1 = bound_index(src_c + 1, src.width);

            unsigned char *pixel_00 = get_pixel_ptr(&src, src_r, src_c);
            unsigned char *pixel_01 = get_pixel_ptr(&src, src_r, src_c_p1);
            unsigned char *pixel_10 = get_pixel_ptr(&src, src_r_p1, src_c);
            unsigned char *pixel_11 = get_pixel_ptr(&src, src_r_p1, src_c_p1);

            unsigned char *pixel = get_pixel_ptr(dst, r, c);
            for (k = 0; k < channels; k++) {
                double v00 = pixel_00[k];
                double v01 = pixel_01[k];
                double v10 = pixel_10[k];
                double v11 = pixel_11[k];
                double value = bilinear_interp(v00, v01, v10, v11, drow, dcol);
                pixel[k] = (unsigned char)(value + 0.5);
            }
        }
    }
    free(src.buffer);
}

/* private blit functions */

static void blit_truecolor(image_t *src, image_t *dst, int swap_rb) {
    int r, c;

    assert(dst->channels == 3 || dst->channels == 4);

    memset(dst->buffer, 0, get_buffer_size(dst));
    for (r = 0; r < src->height && r < dst->height; r++) {
        for (c = 0; c < src->width && c < dst->width; c++) {
            unsigned char *src_pixel = get_pixel_ptr(src, r, c);
            unsigned char *dst_pixel = get_pixel_ptr(dst, r, c);
            if (src->channels == 1 || src->channels == 2) {  /* gray */
                unsigned char gray = src_pixel[0];
                dst_pixel[0] = dst_pixel[1] = dst_pixel[2] = gray;
            } else {
                if (swap_rb) {  /* rgb */
                    dst_pixel[0] = src_pixel[2];
                    dst_pixel[1] = src_pixel[1];
                    dst_pixel[2] = src_pixel[0];
                } else {        /* bgr */
                    dst_pixel[0] = src_pixel[0];
                    dst_pixel[1] = src_pixel[1];
                    dst_pixel[2] = src_pixel[2];
                }
            }
        }
    }
}

void image_blit_bgr(image_t *src, image_t *dst) {
    int swap_rb = 0;
    blit_truecolor(src, dst, swap_rb);
}

void image_blit_rgb(image_t *src, image_t *dst) {
    int swap_rb = 1;
    blit_truecolor(src, dst, swap_rb);
}

/* geometry drawing */

static point_t make_point(int row, int col) {
    point_t point;
    point.row = row;
    point.col = col;
    return point;
}

static void swap_points(point_t *a, point_t *b) {
    point_t t = *a;
    *a = *b;
    *b = t;
}

static void assert_point_valid(point_t point, image_t *image) {
    assert(point.row >= 0 && point.row < image->height);
    assert(point.col >= 0 && point.col < image->width);
}

static int linear_interp_int(int v0, int v1, double d) {
    return (int)(v0 + (v1 - v0) * d + 0.5);
}

static point_t linear_interp_point(point_t p0, point_t p1, double d) {
    point_t point;
    point.row = linear_interp_int(p0.row, p1.row, d);
    point.col = linear_interp_int(p0.col, p1.col, d);
    return point;
}

void image_draw_point(image_t *image, color_t color, point_t point) {
    assert_point_valid(point, image);
    image_set_color(image, point.row, point.col, color);
}

void image_draw_line(image_t *image, color_t color,
                     point_t point0, point_t point1) {
    int row_distance = abs(point1.row - point0.row);
    int col_distance = abs(point1.col - point0.col);
    assert_point_valid(point0, image);
    assert_point_valid(point1, image);
    if (row_distance == 0 && col_distance == 0) {
        image_draw_point(image, color, point0);
    } else if (row_distance > col_distance) {
        int row;
        if (point0.row > point1.row) {
            swap_points(&point0, &point1);
        }
        for (row = point0.row; row <= point1.row; row++) {
            double d = (row - point0.row) / (double)row_distance;
            int col = linear_interp_int(point0.col, point1.col, d);
            image_draw_point(image, color, make_point(row, col));
        }
    } else {
        int col;
        if (point0.col > point1.col) {
            swap_points(&point0, &point1);
        }
        for (col = point0.col; col <= point1.col; col++) {
            double d = (col - point0.col) / (double)col_distance;
            int row = linear_interp_int(point0.row, point1.row, d);
            image_draw_point(image, color, make_point(row, col));
        }
    }
}

void image_draw_triangle(image_t *image, color_t color,
                         point_t point0, point_t point1, point_t point2) {
    assert_point_valid(point0, image);
    assert_point_valid(point1, image);
    assert_point_valid(point2, image);
    image_draw_line(image, color, point0, point1);
    image_draw_line(image, color, point1, point2);
    image_draw_line(image, color, point2, point0);
}

static void sort_points_by_row(point_t *p0, point_t *p1, point_t *p2) {
    if (p0->row > p1->row) {
        swap_points(p0, p1);
    }
    if (p0->row > p2->row) {
        swap_points(p0, p2);
    }
    if (p1->row > p2->row) {
        swap_points(p1, p2);
    }
}

static void sort_points_by_col(point_t *p0, point_t *p1, point_t *p2) {
    if (p0->col > p1->col) {
        swap_points(p0, p1);
    }
    if (p0->col > p2->col) {
        swap_points(p0, p2);
    }
    if (p1->col > p2->col) {
        swap_points(p1, p2);
    }
}

static void draw_scanline(image_t *image, color_t color,
                          point_t p0, point_t p1) {
    point_t point;
    assert(p0.row == p1.row);
    if (p0.col > p1.col) {
        swap_points(&p0, &p1);
    }
    for (point = p0; point.col <= p1.col; point.col++) {
        image_draw_point(image, color, point);
    }
}

void image_fill_triangle(image_t *image, color_t color,
                         point_t point0, point_t point1, point_t point2) {
    assert_point_valid(point0, image);
    assert_point_valid(point1, image);
    assert_point_valid(point2, image);
    sort_points_by_row(&point0, &point1, &point2);
    if (point0.row == point2.row) {
        sort_points_by_col(&point0, &point1, &point2);
        draw_scanline(image, color, point0, point2);
    } else {
        int total_height = point2.row - point0.row;
        int upper_height = point1.row - point0.row;
        int lower_height = point2.row - point1.row;

        /* fill the upper triangle */
        if (upper_height == 0) {
            draw_scanline(image, color, point0, point1);
        } else {
            int row;
            for (row = point0.row; row <= point1.row; row++) {
                double d1 = (row - point0.row) / (double)upper_height;
                double d2 = (row - point0.row) / (double)total_height;
                point_t p1 = linear_interp_point(point0, point1, d1);
                point_t p2 = linear_interp_point(point0, point2, d2);
                p1.row = p2.row = row;
                draw_scanline(image, color, p1, p2);
            }
        }

        /* fill the lower triangle */
        if (lower_height == 0) {
            draw_scanline(image, color, point1, point2);
        } else {
            int row;
            for (row = point1.row; row <= point2.row; row++) {
                double d0 = (row - point0.row) / (double)total_height;
                double d1 = (row - point1.row) / (double)lower_height;
                point_t p0 = linear_interp_point(point0, point2, d0);
                point_t p1 = linear_interp_point(point1, point2, d1);
                p0.row = p1.row = row;
                draw_scanline(image, color, p0, p1);
            }
        }
    }
}
