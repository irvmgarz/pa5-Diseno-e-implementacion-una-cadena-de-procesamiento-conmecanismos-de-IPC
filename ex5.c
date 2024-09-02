#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"
#include "filter.h"

int main(int argc, char **argv) {
    FILE* source;
    FILE* dest;
    BMP_Image* image = NULL;
    BMP_Image* blurredImage = NULL;

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

    blurredImage = createBMPImageFromTemplate(image);
    if (blurredImage == NULL) {
        printError(MEMORY_ERROR);
        freeImage(image);
        fclose(source);
        fclose(dest);
        exit(EXIT_FAILURE);
    }

    apply(image, blurredImage);

    writeImage(argv[2], blurredImage);

    freeImage(image);
    freeImage(blurredImage);
    fclose(source);
    fclose(dest);

    exit(EXIT_SUCCESS);
}
