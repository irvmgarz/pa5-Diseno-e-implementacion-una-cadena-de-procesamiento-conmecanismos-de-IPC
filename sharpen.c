#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "bmp.h"

void applyEdgeDetection(BMP_Image* image) {
    int width = image->header.width_px;
    int height = image->norm_height;
    int halfHeight = height / 2;

    // Crear una imagen temporal para almacenar los resultados del filtro
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

            // Aplicar el kernel de convolución
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

            // Normalizar los valores calculados y asegurarse de que estén en el rango [0, 255]
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

    // Liberar la memoria de la imagen temporal
    freeImage(edgeImage);
}

int main() {
    const char* original_shared_memory_name = "bmp_shared_memory";
    const char* sharpened_shared_memory_name = "bmp_sharpened_memory";
    const int shared_memory_size = sizeof(BMP_Header) + 1920 * 1080 * 4; // Ajusta según el tamaño de la imagen

    // Abrir la memoria compartida original (creada por publisher)
    int shm_fd_original = shm_open(original_shared_memory_name, O_RDONLY, 0666);
    if (shm_fd_original == -1) {
        perror("Error al abrir la memoria compartida original");
        exit(EXIT_FAILURE);
    }

    // Crear nuevo bloque de memoria compartida para la imagen realzada
    int shm_fd_sharpened = shm_open(sharpened_shared_memory_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd_sharpened == -1) {
        perror("Error al crear la memoria compartida para realzado");
        exit(EXIT_FAILURE);
    }

    // Ajustar el tamaño del nuevo bloque de memoria
    ftruncate(shm_fd_sharpened, shared_memory_size);

    // Mapear la memoria original y la nueva memoria para el realzado
    void* shared_memory_original = mmap(0, shared_memory_size, PROT_READ, MAP_SHARED, shm_fd_original, 0);
    if (shared_memory_original == MAP_FAILED) {
        perror("Error al mapear la memoria original");
        exit(EXIT_FAILURE);
    }

    void* shared_memory_sharpened = mmap(0, shared_memory_size, PROT_WRITE, MAP_SHARED, shm_fd_sharpened, 0);
    if (shared_memory_sharpened == MAP_FAILED) {
        perror("Error al mapear la memoria para realzado");
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

    // Crear una imagen nueva para la imagen realzada en la nueva memoria compartida
    BMP_Image sharpened_image;
    sharpened_image.header = original_image.header;
    sharpened_image.norm_height = original_image.norm_height;
    sharpened_image.bytes_per_pixel = original_image.bytes_per_pixel;

    // Configurar los píxeles de la nueva imagen realzada en la nueva memoria compartida
    sharpened_image.pixels = (Pixel**)(malloc(sharpened_image.norm_height * sizeof(Pixel*)));
    uint8_t* sharpened_pixelArray = (uint8_t*)shared_memory_sharpened + sizeof(BMP_Header);
    for (int i = 0; i < sharpened_image.norm_height; i++) {
        sharpened_image.pixels[i] = (Pixel*)(sharpened_pixelArray + i * sharpened_image.header.width_px * sharpened_image.bytes_per_pixel);
    }

    // Copiar la imagen original a la nueva imagen (antes de aplicar el realzado)
    for (int i = 0; i < sharpened_image.norm_height; i++) {
        for (int j = 0; j < sharpened_image.header.width_px; j++) {
            sharpened_image.pixels[i][j] = original_image.pixels[i][j];
        }
    }

    // Aplicar el filtro de realzado de bordes en la parte inferior de la imagen copiada
    applyEdgeDetection(&sharpened_image);

    // Guardar la imagen realzada si se especifica un archivo de salida (opcional)
    if (access("outputs", F_OK) == -1) {
        // Crear directorio 'outputs' si no existe
        mkdir("outputs", 0755);
    }
    writeImage("outputs/sharpened_result.bmp", &sharpened_image);

    // Liberar la memoria utilizada para los punteros de píxeles
    free(original_image.pixels);
    free(sharpened_image.pixels);

    return 0;
}
