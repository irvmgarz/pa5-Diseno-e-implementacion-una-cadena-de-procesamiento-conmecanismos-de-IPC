#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "bmp.h"
#include "filter.h"

int main(int argc, char **argv) {
    const char* shared_memory_name = "bmp_shared_memory";
    const char* new_shared_memory_name = "processed_bmp_blurred_memory";
    const int shared_memory_size = sizeof(BMP_Header) + 1920 * 1080 * 4; // Ajusta según el tamaño de la imagen

    // Abrir la memoria compartida para leer la imagen
    int shm_fd = shm_open(shared_memory_name, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Error al abrir la memoria compartida");
        exit(EXIT_FAILURE);
    }

    // Mapear la memoria compartida
    void* shared_memory = mmap(0, shared_memory_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("Error al mapear la memoria compartida");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    // Acceder a la cabecera y los datos de la imagen desde la memoria compartida
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

    // Aplicar el filtro de desenfoque (Blur)
    applyBlur(&image);

    // Guardar la imagen desenfocada de vuelta al archivo si se proporciona un nombre de archivo
    if (argc == 2) {
        writeImage(argv[1], &image);
    }

    // Crear nueva memoria compartida para la imagen desenfocada
    int new_shm_fd = shm_open(new_shared_memory_name, O_CREAT | O_RDWR, 0666);
    if (new_shm_fd == -1) {
        perror("Error al crear la nueva memoria compartida");
        free(image.pixels);
        munmap(shared_memory, shared_memory_size);
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    // Asignar tamaño a la nueva memoria compartida
    if (ftruncate(new_shm_fd, shared_memory_size) == -1) {
        perror("Error al ajustar el tamaño de la nueva memoria compartida");
        close(new_shm_fd);
        free(image.pixels);
        munmap(shared_memory, shared_memory_size);
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    // Mapear la nueva memoria compartida
    void* new_shared_memory = mmap(0, shared_memory_size, PROT_READ | PROT_WRITE, MAP_SHARED, new_shm_fd, 0);
    if (new_shared_memory == MAP_FAILED) {
        perror("Error al mapear la nueva memoria compartida");
        close(new_shm_fd);
        free(image.pixels);
        munmap(shared_memory, shared_memory_size);
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    // Copiar la imagen desenfocada a la nueva memoria compartida
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
