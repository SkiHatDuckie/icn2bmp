#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHUNK_SIZE 8

unsigned char* read_contents(char* filename, size_t buf_size) {
    FILE *file_ptr = fopen(filename, "rb");
    if (!file_ptr) {
        fprintf(stderr, "Unable to open file\n");
        exit(EXIT_FAILURE);
    }

    unsigned char *buffer = malloc(buf_size);
    fread(buffer, buf_size, 1, file_ptr);
    fclose(file_ptr);
    return buffer;
}

unsigned char reverse_bits(unsigned char b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

unsigned char* format_data(int w, int h, size_t buf_size, unsigned char *buffer) {
    unsigned char *data = malloc(buf_size);
    for (int i = 0; i < w * h; i++) {
        int row_offset = i / w * w * CHUNK_SIZE;
        int col_offset = i % w;
        for (int chunk_idx = 0; chunk_idx < CHUNK_SIZE; chunk_idx++) {
            int data_idx = chunk_idx * w + col_offset + row_offset;
            int buf_idx = buf_size - (chunk_idx + i * CHUNK_SIZE) - 1;
            data[data_idx] = reverse_bits(buffer[buf_idx]);
        }
    }
    return data;
}

void remove_extension(char *filename) {
    filename[strlen(filename)-4] = 0;
}

void create_bmp(int w, int h, size_t buf_size, unsigned char *data, char *filename) {
    char tag[] = { 'B', 'M' };
    int header[] = {
        0x00,                // File size
        0x00,                // Unused
        0x3e,                // Byte offset of pixel data
        0x28,                // Header size
        w, h,                // Image dimensions in pixels
        0x010001,            // 1 bit/pixel, 1 color plane
        0x00,                // BI_RGB no compression
        0x00,                // Pixel data size in bytes
        0x00, 0x00,          // Print resolution
        0x00, 0x00           // Color palette
    };
    int color_table[] = { 0xFFFFFF, 0x000000 };
    header[0] = sizeof tag + sizeof header + sizeof color_table + buf_size;

    FILE *file_ptr = fopen(strcat(filename, ".bmp"), "wb");
    fwrite(&tag, sizeof tag, 1, file_ptr);
    fwrite(&header, sizeof header, 1, file_ptr);
    fwrite(&color_table, sizeof color_table, 1, file_ptr);
    fwrite(data, buf_size, 1, file_ptr);
    fclose(file_ptr);
}

int main(int argc, char *argv[]) {
    // Check args
    if (argc != 4) {
        fprintf(stderr, "Usage: icn2bmp [width] [height] [file]\n");
        exit(EXIT_FAILURE);
    }

    // Read non-optional arguments
    const int width = atoi(argv[1]);  // Width of .icn in chunks
    const int height = atoi(argv[2]);  // Height of .icn in chunks
    char *filename = argv[3];

    // Read file to buffer
    const size_t buf_size = width * height * CHUNK_SIZE;
    unsigned char *buffer = read_contents(filename, buf_size);

    // Reformat pointers from chunk order into col-row order
    unsigned char *data = format_data(width, height, buf_size, buffer);
    free(buffer);

    // Create new .bmp
    remove_extension(filename);
    create_bmp(width * CHUNK_SIZE, height * CHUNK_SIZE, buf_size, data, filename);
    free(data);

    return 0;
}