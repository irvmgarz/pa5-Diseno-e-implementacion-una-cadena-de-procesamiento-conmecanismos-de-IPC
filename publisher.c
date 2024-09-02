#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <string.h>
#include "bmp.h"
#include <unistd.h> // Para obtener el tamaño de página

int main() {
    key_t key = ftok("shmfile", 65); 
    if (key == -1) {
        perror("ftok error");
        exit(1);
    }

    char image_path[256];
    printf("Ingrese la ruta de la imagen BMP: ");
    scanf("%s", image_path);

    FILE *srcFile = fopen(image_path, "rb");
    if (srcFile == NULL) {
        printError(FILE_ERROR);
        exit(1);
    }

    BMP_Image *image = NULL;
    readImage(srcFile, &image);
    fclose(srcFile);

    if (image == NULL) {
        printError(VALID_ERROR);
        exit(1);
    }

    // Calcular SHM_SIZE basado en el tamaño real de la imagen
    size_t image_size = image->header.width_px * abs(image->header.height_px) * (image->header.bits_per_pixel / 8);
    size_t page_size = sysconf(_SC_PAGESIZE);
    size_t SHM_SIZE = ((image_size + page_size - 1) / page_size) * page_size; // Redondear al siguiente múltiplo de la página

    int shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT);
    if (shmid < 0) {
        perror("shmget error");
        exit(1);
    }

    char *shmaddr = (char*) shmat(shmid, (void*)0, 0);
    if (shmaddr == (char *) -1) {
        perror("shmat error");
        exit(1);
    }

    memcpy(shmaddr, image->pixels, image->header.imagesize); // Copiar imagen a memoria compartida
    printf("Imagen cargada en memoria compartida.\n");

    freeImage(image);
    shmdt(shmaddr); 
    return 0;
}
