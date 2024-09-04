#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "bmp.h"
#include "filter.h"

int main(int argc, char **argv) {
    const char* shared_memory_name = "bmp_shared_memory";
    const int shared_memory_size = sizeof(BMP_Header) + 1920 * 1080 * 4; // Ajusta según el tamaño de la imagen

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

    // Aplicar el filtro de desenfoque a la imagen cargada en memoria compartida
    applyBlur(&image);

    // Guardar la imagen desenfocada de vuelta al archivo (si es necesario)
    if (argc == 2) {
        writeImage(argv[1], &image);
    }

    // Liberar la memoria utilizada para los punteros de píxeles
    free(image.pixels);

    return 0;
}
