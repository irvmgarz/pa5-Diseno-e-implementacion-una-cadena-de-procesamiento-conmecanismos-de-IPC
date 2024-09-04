#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "bmp.h"

void applyEdgeDetection(BMP_Image* image) {
    int width = image->header.width_px;
    int height = image->norm_height;
    int halfHeight = height / 2;

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

            // Limitar los valores a 0-255 para evitar distorsión de color
            image->pixels[i][j].blue = (sumBlue < 0) ? 0 : (sumBlue > 255) ? 255 : sumBlue;
            image->pixels[i][j].green = (sumGreen < 0) ? 0 : (sumGreen > 255) ? 255 : sumGreen;
            image->pixels[i][j].red = (sumRed < 0) ? 0 : (sumRed > 255) ? 255 : sumRed;
        }
    }
}

int main() {
    const char* shared_memory_name = "bmp_shared_memory";
    const int shared_memory_size = sizeof(BMP_Header) + 1920 * 1080 * 4; // Ajusta según el tamaño de la imagen

    // Abrir memoria compartida
    int shm_fd = shm_open(shared_memory_name, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Error al abrir la memoria compartida");
        exit(EXIT_FAILURE);
    }

    void* shared_memory = mmap(0, shared_memory_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("Error al mapear la memoria compartida");
        exit(EXIT_FAILURE);
    }

    // Acceder a la cabecera y a los datos de la imagen desde la memoria compartida
    BMP_Image image;
    image.header = *(BMP_Header*)shared_memory;
    image.norm_height = abs(image.header.height_px);
    image.bytes_per_pixel = image.header.bits_per_pixel / 8;

    // Configurar los píxeles de la imagen para que apunten a la memoria compartida
    image.pixels = (Pixel**)(malloc(image.norm_height * sizeof(Pixel*)));
    uint8_t* pixelArray = (uint8_t*)shared_memory + sizeof(BMP_Header);
    for (int i = 0; i < image.norm_height; i++) {
        image.pixels[i] = (Pixel*)(pixelArray + i * image.header.width_px * image.bytes_per_pixel);
    }

    // Aplicar el filtro de realzado a la imagen cargada en memoria compartida
    applyEdgeDetection(&image);

    // Guardar la imagen procesada en la carpeta "outputs" como resultado.bmp
    char outputFilePath[256] = "outputs/resultado.bmp";
    writeImage(outputFilePath, &image);

    // Liberar la memoria utilizada para los punteros de píxeles
    free(image.pixels);

    printf("Imagen realzada guardada en %s\n", outputFilePath);

    return 0;
}
