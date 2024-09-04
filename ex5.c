#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "bmp.h"
#include "filter.h"

void makeBottomTransparent(BMP_Image* image) {
    int width = image->header.width_px;
    int height = image->norm_height;
    int halfHeight = height / 2;

    // Hacer la mitad inferior de la imagen transparente
    for (int i = halfHeight; i < height; i++) {
        for (int j = 0; j < width; j++) {
            image->pixels[i][j].alpha = 0;  // Establecer transparencia total
            image->pixels[i][j].blue = 0;   // Eliminar los canales de color
            image->pixels[i][j].green = 0;
            image->pixels[i][j].red = 0;
        }
    }
}

int main(int argc, char **argv) {
    const char* original_shared_memory_name = "bmp_shared_memory";
    const char* blurred_shared_memory_name = "bmp_blurred_memory";
    const int shared_memory_size = sizeof(BMP_Header) + 1920 * 1080 * 4; // Ajusta según el tamaño de la imagen

    // Abrir la memoria compartida original (creada por publisher)
    int shm_fd_original = shm_open(original_shared_memory_name, O_RDONLY, 0666);
    if (shm_fd_original == -1) {
        perror("Error al abrir la memoria compartida original");
        exit(EXIT_FAILURE);
    }

    // Crear nuevo bloque de memoria compartida para la imagen desenfocada
    int shm_fd_blurred = shm_open(blurred_shared_memory_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd_blurred == -1) {
        perror("Error al crear la memoria compartida para desenfoque");
        exit(EXIT_FAILURE);
    }

    // Ajustar el tamaño del nuevo bloque de memoria
    ftruncate(shm_fd_blurred, shared_memory_size);

    // Mapear la memoria original y la nueva memoria para el desenfoque
    void* shared_memory_original = mmap(0, shared_memory_size, PROT_READ, MAP_SHARED, shm_fd_original, 0);
    if (shared_memory_original == MAP_FAILED) {
        perror("Error al mapear la memoria original");
        exit(EXIT_FAILURE);
    }

    void* shared_memory_blurred = mmap(0, shared_memory_size, PROT_WRITE, MAP_SHARED, shm_fd_blurred, 0);
    if (shared_memory_blurred == MAP_FAILED) {
        perror("Error al mapear la memoria para desenfoque");
        exit(EXIT_FAILURE);
    }

    // Acceder a la cabecera y los datos de la imagen desde la memoria original
    BMP_Image original_image;
    original_image.header = *(BMP_Header*)shared_memory_original;
    original_image.norm_height = abs(original_image.header.height_px);
    original_image.bytes_per_pixel = original_image.header.bits_per_pixel / 8;

    // Configurar los píxeles de la imagen original
    original_image.pixels = (Pixel**)(malloc(original_image.norm_height * sizeof(Pixel*)));
    uint8_t* original_pixelArray = (uint8_t*)shared_memory_original + sizeof(BMP_Header);
    for (int i = 0; i < original_image.norm_height; i++) {
        original_image.pixels[i] = (Pixel*)(original_pixelArray + i * original_image.header.width_px * original_image.bytes_per_pixel);
    }

    // Crear una imagen nueva para la imagen desenfocada en la nueva memoria compartida
    BMP_Image blurred_image;
    blurred_image.header = original_image.header;
    blurred_image.norm_height = original_image.norm_height;
    blurred_image.bytes_per_pixel = original_image.bytes_per_pixel;

    // Configurar los píxeles de la nueva imagen desenfocada en la nueva memoria compartida
    blurred_image.pixels = (Pixel**)(malloc(blurred_image.norm_height * sizeof(Pixel*)));
    uint8_t* blurred_pixelArray = (uint8_t*)shared_memory_blurred + sizeof(BMP_Header);
    for (int i = 0; i < blurred_image.norm_height; i++) {
        blurred_image.pixels[i] = (Pixel*)(blurred_pixelArray + i * blurred_image.header.width_px * blurred_image.bytes_per_pixel);
    }

    // Copiar la imagen original a la nueva imagen (antes de aplicar el desenfoque)
    for (int i = 0; i < blurred_image.norm_height; i++) {
        for (int j = 0; j < blurred_image.header.width_px; j++) {
            blurred_image.pixels[i][j] = original_image.pixels[i][j];
        }
    }

    // Hacer transparente la parte inferior de la imagen
    makeBottomTransparent(&blurred_image);

    // Aplicar el filtro de desenfoque a la imagen copiada en la nueva memoria compartida
    applyBlur(&blurred_image);

    // Guardar la imagen desenfocada si se especifica un archivo de salida
    if (argc == 2) {
        writeImage(argv[1], &blurred_image);
    }

    // Liberar la memoria utilizada para los punteros de píxeles
    free(original_image.pixels);
    free(blurred_image.pixels);

    return 0;
}
