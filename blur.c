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

    printf("Aplicando filtro de desenfoque...\n");

    int boxFilter[3][3] = {
        {1, 1, 1},
        {1, 1, 1},
        {1, 1, 1}
    };

    applyParallel(imageIn, imageOut, boxFilter, 4); // Aplicar desenfoque con 4 hilos

    memcpy(shmaddr, imageOut->pixels, imageOut->header.imagesize); // Guardar la imagen procesada en memoria compartida

    printf("Desenfoque aplicado a la primera mitad de la imagen.\n");

    freeImage(imageIn);
    freeImage(imageOut);
    shmdt(shmaddr); 
    return 0;
}
