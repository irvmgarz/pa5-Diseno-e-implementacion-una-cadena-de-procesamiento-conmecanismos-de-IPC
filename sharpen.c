#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"

void applyEdgeDetection(BMP_Image* image) {
    int width = image->header.width_px;
    int height = image->norm_height;
    int halfHeight = height / 2;

    // Crear una copia de la imagen para aplicar el filtro de bordes
    BMP_Image* edgeImage = createBMPImageFromTemplate(image);
    if (edgeImage == NULL) {
        return;
    }

    // Definir el kernel de realzado de bordes
    int edgeKernel[3][3] = {
        {-1, -1, -1},
        {-1,  8, -1},
        {-1, -1, -1}
    };

    // Aplicar el filtro de realzado de bordes solo en la mitad inferior
    for (int i = halfHeight; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int sumBlue = 0, sumGreen = 0, sumRed = 0;

            // Aplicar el kernel
            for (int di = -1; di <= 1; di++) {
                for (int dj = -1; dj <= 1; dj++) {
                    int ni = i + di;
                    int nj = j + dj;
                    if (ni >= halfHeight && ni < height && nj >= 0 && nj < width) {
                        sumBlue += image->pixels[ni][nj].blue * edgeKernel[di + 1][dj + 1];
                        sumGreen += image->pixels[ni][nj].green * edgeKernel[di + 1][dj + 1];
                        sumRed += image->pixels[ni][nj].red * edgeKernel[di + 1][dj + 1];
                    }
                }
            }

            // Asignar los valores calculados al píxel correspondiente
            edgeImage->pixels[i][j].blue = (sumBlue < 0) ? 0 : (sumBlue > 255) ? 255 : sumBlue;
            edgeImage->pixels[i][j].green = (sumGreen < 0) ? 0 : (sumGreen > 255) ? 255 : sumGreen;
            edgeImage->pixels[i][j].red = (sumRed < 0) ? 0 : (sumRed > 255) ? 255 : sumRed;
        }
    }

    // Copiar la mitad inferior con bordes realzados de vuelta a la imagen original
    for (int i = halfHeight; i < height; i++) {
        for (int j = 0; j < width; j++) {
            image->pixels[i][j] = edgeImage->pixels[i][j];
        }
    }

    // Liberar la memoria de la imagen procesada
    freeImage(edgeImage);
}

int main(int argc, char **argv) {
    FILE* source;
    FILE* dest;
    BMP_Image* image = NULL;

    if (argc != 3) {
        printError(ARGUMENT_ERROR);
        exit(EXIT_FAILURE);
    }
    
    if ((source = fopen(argv[1], "rb")) == NULL) {
        printError(FILE_ERROR);
        exit(EXIT_FAILURE);
    }
    if ((dest = fopen(argv[2], "wb")) == NULL) {
        printError(FILE_ERROR);
        fclose(source);
        exit(EXIT_FAILURE);
    } 

    readImage(source, &image);

    if (!checkBMPValid(&image->header)) {
        printError(VALID_ERROR);
        freeImage(image);
        fclose(source);
        fclose(dest);
        exit(EXIT_FAILURE);
    }

    // Aplicar el filtro de realzado de bordes en la mitad inferior de la imagen
    applyEdgeDetection(image);

    // Escribir la imagen procesada a la salida
    writeImage(argv[2], image);

    // Limpiar y cerrar archivos
    freeImage(image);
    fclose(source);
    fclose(dest);

    exit(EXIT_SUCCESS);
}
