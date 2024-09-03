#include <stdlib.h>
#include <stdio.h>
#include "bmp.h"

void applyBlur(BMP_Image* image) {
    int width = image->header.width_px;
    int height = image->norm_height;
    int halfHeight = height / 2;

    // Definir el radio del desenfoque
    int blurRadius = 1;

    // Crear una copia de la imagen para aplicar el desenfoque
    BMP_Image* blurredImage = createBMPImageFromTemplate(image);
    if (blurredImage == NULL) {
        return;
    }

    // Aplicar el desenfoque solo en la mitad superior
    for (int i = 0; i < halfHeight; i++) {  // Solo procesar filas en la mitad superior
        for (int j = 0; j < width; j++) {
            int sumBlue = 0, sumGreen = 0, sumRed = 0;
            int count = 0;

            // Promediar los píxeles dentro del radio de desenfoque
            for (int di = -blurRadius; di <= blurRadius; di++) {
                for (int dj = -blurRadius; dj <= blurRadius; dj++) {
                    int ni = i + di;
                    int nj = j + dj;
                    if (ni >= 0 && ni < halfHeight && nj >= 0 && nj < width) {  // Limitar a la mitad superior
                        sumBlue += image->pixels[ni][nj].blue;
                        sumGreen += image->pixels[ni][nj].green;
                        sumRed += image->pixels[ni][nj].red;
                        count++;
                    }
                }
            }

            // Asignar los valores promediados al píxel actual en la imagen desenfocada
            blurredImage->pixels[i][j].blue = sumBlue / count;
            blurredImage->pixels[i][j].green = sumGreen / count;
            blurredImage->pixels[i][j].red = sumRed / count;
        }
    }

    // Solo copiar la mitad superior desenfocada de vuelta a la imagen original
    for (int i = 0; i < halfHeight; i++) {
        for (int j = 0; j < width; j++) {
            image->pixels[i][j] = blurredImage->pixels[i][j];
        }
    }

    // Liberar la memoria de la imagen desenfocada
    freeImage(blurredImage);
}


void printError(int error) {
    switch(error) {
    case ARGUMENT_ERROR:
        printf("Usage: ex5 <source> <destination>\n");
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
    BMP_Image* image = (BMP_Image*)malloc(sizeof(BMP_Image));
    if (image == NULL) {
        printError(MEMORY_ERROR);
        return NULL;
    }

    fread(&(image->header), sizeof(BMP_Header), 1, fptr);

    image->norm_height = abs(image->header.height_px);
    image->bytes_per_pixel = image->header.bits_per_pixel / 8;

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

    return image;
}

BMP_Image* createBMPImageFromTemplate(const BMP_Image* template) {  // Usar const aquí
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
    fseek(srcFile, image->header.offset, SEEK_SET);
    for (int i = 0; i < image->norm_height; i++) {
        fread(image->pixels[i], image->bytes_per_pixel, image->header.width_px, srcFile);
    }
}

void readImage(FILE *srcFile, BMP_Image **dataImage) {
    *dataImage = createBMPImage(srcFile);
    if (*dataImage == NULL) {
        return;
    }
    readImageData(srcFile, *dataImage, (*dataImage)->header.image_size);  // Cambio a image_size
}

void writeImage(const char* destFileName, BMP_Image* dataImage) {  // Usar const aquí
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

int checkBMPValid(const BMP_Header* header) {  // Usar const aquí
    if (header->type != 0x4D42) {
        return 0;
    }
    if (header->bits_per_pixel != 24 && header->bits_per_pixel != 32) {
        return 0;
    }
    return 1;
}
