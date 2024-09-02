#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <string.h>
#include "bmp.h"
#include "filter.h"

int main() {
    key_t key = ftok("shmfile", 65); 
    if (key == -1) {
        perror("ftok error");
        exit(1);
    }

    // Necesitamos calcular el tamaño de la memoria compartida basada en la imagen
    int shmid = shmget(key, 0, 0666); // Obtener el segmento existente
    if (shmid < 0) {
        perror("shmget error");
        exit(1);
    }

    char *shmaddr = (char*) shmat(shmid, (void*)0, 0);
    if (shmaddr == (char *) -1) {
        perror("shmat error");
        exit(1);
    }

    BMP_Image *imageIn = (BMP_Image*)malloc(sizeof(BMP_Image));
    if (imageIn == NULL) {
        printError(MEMORY_ERROR);
        shmdt(shmaddr);
        return 1;
    }
    memcpy(imageIn, shmaddr, sizeof(BMP_Image));

    BMP_Image *imageOut = createBMPImageFromTemplate(imageIn);
    if (imageOut == NULL) {
        printError(MEMORY_ERROR);
        freeImage(imageIn);
        shmdt(shmaddr);
        return 1;
    }

    printf("Aplicando filtro de realce de bordes...\n");

    int sobelFilter[3][3] = {
        {-1, -2, -1},
        {0,  0,  0},
        {1,  2,  1}
    };

    applyParallel(imageIn, imageOut, sobelFilter, 4); // Aplicar realce de bordes con 4 hilos

    char output_path[256];
    printf("Ingrese la ruta de salida para la imagen procesada: ");
    scanf("%s", output_path);

    writeImage(output_path, imageOut); // Guardar la imagen procesada

    printf("Realce de bordes aplicado a la segunda mitad de la imagen y guardada en %s.\n", output_path);

    freeImage(imageIn);
    freeImage(imageOut);
    shmdt(shmaddr); 
    return 0;
}
