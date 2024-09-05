#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "bmp.h"
#include <string.h>

void applyEdgeDetection(BMP_Image* image) {
    int width = image->header.width_px;
    int height = image->norm_height;
    int halfHeight = height / 2;

    BMP_Image* edgeImage = createBMPImageFromTemplate(image);
    if (edgeImage == NULL) {
        return;
    }

    int edgeKernel[3][3] = {
        {-1, -1, -1},
        {-1,  8, -1},
        {-1, -1, -1}
    };

    for (int i = halfHeight; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int sumBlue = 0, sumGreen = 0, sumRed = 0;

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

            edgeImage->pixels[i][j].blue = (sumBlue < 0) ? 0 : (sumBlue > 255) ? 255 : sumBlue;
            edgeImage->pixels[i][j].green = (sumGreen < 0) ? 0 : (sumGreen > 255) ? 255 : sumGreen;
            edgeImage->pixels[i][j].red = (sumRed < 0) ? 0 : (sumRed > 255) ? 255 : sumRed;
        }
    }

    for (int i = halfHeight; i < height; i++) {
        for (int j = 0; j < width; j++) {
            image->pixels[i][j] = edgeImage->pixels[i][j];
        }
    }

    freeImage(edgeImage);
}

int main() {
    const char* shared_memory_name = "bmp_shared_memory";
    const int shared_memory_size = sizeof(BMP_Header) + 1920 * 1080 * 4;

    int shm_fd = shm_open(shared_memory_name, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Error al abrir la memoria compartida");
        exit(EXIT_FAILURE);
    }

    void* shared_memory = mmap(0, shared_memory_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("Error al mapear la memoria compartida");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    BMP_Image image;
    image.header = *(BMP_Header*)shared_memory;
    image.norm_height = abs(image.header.height_px);
    image.bytes_per_pixel = image.header.bits_per_pixel / 8;

    image.pixels = (Pixel**)(malloc(image.norm_height * sizeof(Pixel*)));
    uint8_t* pixelArray = (uint8_t*)shared_memory + sizeof(BMP_Header);
    for (int i = 0; i < image.norm_height; i++) {
        image.pixels[i] = (Pixel*)(pixelArray + i * image.header.width_px * image.bytes_per_pixel);
    }

    // Aplicar el filtro de realzado de bordes
    applyEdgeDetection(&image);

    // Crear nueva memoria compartida para la imagen procesada
    const char* new_shared_memory_name = "processed_bmp_sharped_memory";  // Cambié el nombre aquí
    int new_shm_fd = shm_open(new_shared_memory_name, O_CREAT | O_RDWR, 0666);
    if (new_shm_fd == -1) {
        perror("Error al crear nueva memoria compartida");
        exit(EXIT_FAILURE);
    }

    // Asignar tamaño a la nueva memoria compartida
    if (ftruncate(new_shm_fd, shared_memory_size) == -1) {
        perror("Error al ajustar el tamaño de la nueva memoria compartida");
        exit(EXIT_FAILURE);
    }

    // Mapear la nueva memoria compartida
    void* new_shared_memory = mmap(0, shared_memory_size, PROT_READ | PROT_WRITE, MAP_SHARED, new_shm_fd, 0);
    if (new_shared_memory == MAP_FAILED) {
        perror("Error al mapear la nueva memoria compartida");
        close(new_shm_fd);
        exit(EXIT_FAILURE);
    }

    // Copiar los datos de la imagen procesada a la nueva memoria compartida
    memcpy(new_shared_memory, shared_memory, shared_memory_size);

    // Liberar la memoria utilizada para los punteros de píxeles
    free(image.pixels);

    // Desmapear y cerrar las memorias compartidas
    munmap(shared_memory, shared_memory_size);
    munmap(new_shared_memory, shared_memory_size);
    close(shm_fd);
    close(new_shm_fd);

    return 0;
}
