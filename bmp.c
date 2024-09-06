#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "bmp.h"

// Estructura para pasar argumentos a cada hilo
typedef struct {
    BMP_Image* image;
    int start_row;
    int end_row;
    int blur_radius;
} ThreadArgs;

// Función que será ejecutada por cada hilo
void* applyBlurThread(void* args) {
    ThreadArgs* threadArgs = (ThreadArgs*)args;
    BMP_Image* image = threadArgs->image;
    int start_row = threadArgs->start_row;
    int end_row = threadArgs->end_row;
    int blurRadius = threadArgs->blur_radius;

    int width = image->header.width_px;

    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < width; j++) {
            int sumBlue = 0, sumGreen = 0, sumRed = 0;
            int count = 0;

            // Aplicar el desenfoque promediando los píxeles dentro del radio
            for (int di = -blurRadius; di <= blurRadius; di++) {
                for (int dj = -blurRadius; dj <= blurRadius; dj++) {
                    int ni = i + di;
                    int nj = j + dj;
                    if (ni >= 0 && ni < end_row && nj >= 0 && nj < width) {
                        sumBlue += image->pixels[ni][nj].blue;
                        sumGreen += image->pixels[ni][nj].green;
                        sumRed += image->pixels[ni][nj].red;
                        count++;
                    }
                }
            }

            // Asignar los valores promediados al píxel actual
            image->pixels[i][j].blue = sumBlue / count;
            image->pixels[i][j].green = sumGreen / count;
            image->pixels[i][j].red = sumRed / count;
        }
    }

    return NULL;
}

// Función para aplicar desenfoque utilizando múltiples hilos
void applyBlurMultiThreaded(BMP_Image* image, int num_threads) {
    int height = image->norm_height / 2;  // Limitar a la mitad superior
    int rows_per_thread = height / num_threads;
    pthread_t threads[num_threads];
    ThreadArgs args[num_threads];

    // Crear hilos para aplicar el desenfoque en paralelo
    for (int i = 0; i < num_threads; i++) {
        int start_row = i * rows_per_thread;
        int end_row = (i == num_threads - 1) ? height : (i + 1) * rows_per_thread;  // El último hilo toma el resto

        args[i].image = image;
        args[i].start_row = start_row;
        args[i].end_row = end_row;
        args[i].blur_radius = 1;  // Puedes ajustar el radio del desenfoque si lo deseas

        pthread_create(&threads[i], NULL, applyBlurThread, &args[i]);
    }

    // Esperar a que todos los hilos terminen
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}

// Función para imprimir errores
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

// Crear una imagen BMP a partir de un archivo
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

// Crear una imagen BMP a partir de otra como plantilla
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

// Leer los datos de la imagen BMP desde un archivo
void readImageData(FILE* srcFile, BMP_Image* image, int dataSize) {
    fseek(srcFile, image->header.offset, SEEK_SET);
    for (int i = 0; i < image->norm_height; i++) {
        fread(image->pixels[i], image->bytes_per_pixel, image->header.width_px, srcFile);
    }
}

// Leer una imagen BMP desde un archivo
void readImage(FILE *srcFile, BMP_Image **dataImage) {
    *dataImage = createBMPImage(srcFile);
    if (*dataImage == NULL) {
        return;
    }
    readImageData(srcFile, *dataImage, (*dataImage)->header.image_size);
}

// Escribir una imagen BMP a un archivo
void writeImage(const char* destFileName, BMP_Image* dataImage) {
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

// Liberar la memoria ocupada por una imagen BMP
void freeImage(BMP_Image* image) {
    for (int i = 0; i < image->norm_height; i++) {
        free(image->pixels[i]);
    }
    free(image->pixels);
    free(image);
}

// Verificar si el archivo BMP es válido
int checkBMPValid(const BMP_Header* header) {
    if (header->type != 0x4D42) {
        return 0;
    }
    if (header->bits_per_pixel != 24 && header->bits_per_pixel != 32) {
        return 0;
    }
    return 1;
}
