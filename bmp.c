#include <stdlib.h>
#include <stdio.h>
#include "bmp.h"

void printError(int error) {
    switch(error) {
    case ARGUMENT_ERROR:
        printf("Usage:ex5 <source> <destination>\n");
        break;
    case FILE_ERROR:
        printf("Unable to open file!\n");
        break;
    case MEMORY_ERROR:
        printf("Unable to allocate memory!\n");
        break;
    case VALID_ERROR:
        printf("BMP file not valid!\n");
        break;
    default:
        break;
    }
}

BMP_Image* createBMPImage(FILE* fptr) {
    printf("Creando imagen BMP...\n");
    BMP_Image* image = (BMP_Image*)malloc(sizeof(BMP_Image));
    if (image == NULL) {
        printError(MEMORY_ERROR);
        return NULL;
    }

    fread(&(image->header), sizeof(BMP_Header), 1, fptr);
    printf("Header leido satisfactoriamente.\n");

    image->norm_height = abs(image->header.height_px);
    image->bytes_per_pixel = image->header.bits_per_pixel / 8;
    int dataSize = image->header.width_px * image->norm_height * image->bytes_per_pixel;

    image->pixels = (Pixel**)malloc(image->norm_height * sizeof(Pixel*));
    if (image->pixels == NULL) {
        printError(MEMORY_ERROR);
        free(image);
        return NULL;
    }

    for (int i = 0; i < image->norm_height; i++) {
        image->pixels[i] = (Pixel*)malloc(image->header.width_px * sizeof(Pixel));
        if (image->pixels[i] == NULL) {
            printError(MEMORY_ERROR);
            for (int j = 0; j < i; j++) {
                free(image->pixels[j]);
            }
            free(image->pixels);
            free(image);
            return NULL;
        }
    }

    printf("Imagen BMP creada satisfactoriamente.\n");
    return image;
}

BMP_Image* createBMPImageFromTemplate(const BMP_Image* template) {
    BMP_Image* image = (BMP_Image*)malloc(sizeof(BMP_Image));
    if (image == NULL) {
        printError(MEMORY_ERROR);
        return NULL;
    }

    image->header = template->header;
    image->norm_height = template->norm_height;
    image->bytes_per_pixel = template->bytes_per_pixel;

    image->pixels = (Pixel**)malloc(image->norm_height * sizeof(Pixel*));
    if (image->pixels == NULL) {
        printError(MEMORY_ERROR);
        free(image);
        return NULL;
    }

    for (int i = 0; i < image->norm_height; i++) {
        image->pixels[i] = (Pixel*)malloc(template->header.width_px * sizeof(Pixel));
        if (image->pixels[i] == NULL) {
            printError(MEMORY_ERROR);
            for (int j = 0; j < i; j++) {
                free(image->pixels[j]);
            }
            free(image->pixels);
            free(image);
            return NULL;
        }
    }

    return image;
}

void readImageData(FILE* srcFile, BMP_Image* image, int dataSize) {
    printf("Leyendo datos de la imagen...\n");
    fseek(srcFile, image->header.offset, SEEK_SET);
    for (int i = 0; i < image->norm_height; i++) {
        fread(image->pixels[i], image->bytes_per_pixel, image->header.width_px, srcFile);
    }
    printf("Datos de la imagen leidos satisfactoriamente.\n");
}

void readImage(FILE *srcFile, BMP_Image **dataImage) {
    *dataImage = createBMPImage(srcFile);
    if (*dataImage == NULL) {
        return;
    }
    readImageData(srcFile, *dataImage, (*dataImage)->header.imagesize);
}

void writeImage(char* destFileName, BMP_Image* dataImage) {
    FILE* destFile = fopen(destFileName, "wb");
    if (destFile == NULL) {
        printError(FILE_ERROR);
        return;
    }

    fwrite(&(dataImage->header), sizeof(BMP_Header), 1, destFile);
    fseek(destFile, dataImage->header.offset, SEEK_SET);
    for (int i = 0; i < dataImage->norm_height; i++) {
        fwrite(dataImage->pixels[i], dataImage->bytes_per_pixel, dataImage->header.width_px, destFile);
    }

    fclose(destFile);
}

void freeImage(BMP_Image* image) {
    for (int i = 0; i < image->norm_height; i++) {
        free(image->pixels[i]);
    }
    free(image->pixels);
    free(image);
}

int checkBMPValid(BMP_Header* header) {
    if (header->type != 0x4d42) {
        return FALSE;
    }
    if (header->bits_per_pixel != 24 && header->bits_per_pixel != 32) {
        return FALSE;
    }
    if (header->planes != 1) {
        return FALSE;
    }
    if (header->compression != 0) {
        return FALSE;
    }
    return TRUE;
}

void printBMPHeader(BMP_Header* header) {
    printf("file type (should be 0x4d42): %x\n", header->type);
    printf("file size: %d\n", header->size);
    printf("offset to image data: %d\n", header->offset);
    printf("header size: %d\n", header->header_size);
    printf("width_px: %d\n", header->width_px);
    printf("height_px: %d\n", header->height_px);
    printf("planes: %d\n", header->planes);
    printf("bits: %d\n", header->bits_per_pixel);
}

void printBMPImage(BMP_Image* image) {
    printf("data size is %ld\n", sizeof(image->pixels));
    printf("norm_height size is %d\n", image->norm_height);
    printf("bytes per pixel is %d\n", image->bytes_per_pixel);
}
