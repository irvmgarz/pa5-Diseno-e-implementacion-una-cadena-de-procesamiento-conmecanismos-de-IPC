#ifndef BMP_H
#define BMP_H

#include <stdint.h>
#include <stdio.h>

typedef struct __attribute__((packed)) BMP_Header {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
    uint32_t header_size;
    int32_t width_px;
    int32_t height_px;
    uint16_t planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t image_size;
    int32_t x_pixels_per_meter;
    int32_t y_pixels_per_meter;
    uint32_t colors_used;
    uint32_t colors_important;
} BMP_Header;

typedef struct __attribute__((packed)) Pixel {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
} Pixel;

typedef struct {
    BMP_Header header;
    int norm_height;
    int bytes_per_pixel;
    Pixel **pixels;
} BMP_Image;

// Definiciones de códigos de error
#define ARGUMENT_ERROR 1
#define FILE_ERROR 2
#define MEMORY_ERROR 3
#define VALID_ERROR 4

// Declaración de funciones
void printError(int error);
BMP_Image* createBMPImage(FILE* fptr);
void readImage(FILE* srcFile, BMP_Image** image);
int checkBMPValid(const BMP_Header* header);  // Cambio aquí para const
BMP_Image* createBMPImageFromTemplate(const BMP_Image* image);  // Cambio aquí para const
void apply(BMP_Image* image, BMP_Image* blurredImage);
void writeImage(const char* filename, BMP_Image* image);  // Cambio aquí para const
void freeImage(BMP_Image* image);
void readImageData(FILE* srcFile, BMP_Image* image, int dataSize);

#endif // BMP_H
