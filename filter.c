#include "bmp.h"
#include <pthread.h>

typedef struct {
    BMP_Image *imageIn;
    BMP_Image *imageOut;
    int startRow;
    int endRow;
    int width;
    int height;
    int (*boxFilter)[3];
} FilterArgs;

void apply(BMP_Image * imageIn, BMP_Image * imageOut) {
    int width = imageIn->header.width_px;
    int height = imageIn->norm_height;

    int boxFilter[3][3] = {
        {1, 1, 1},
        {1, 1, 1},
        {1, 1, 1}
    };
    
    int filterSize = 3;
    int sum = 9;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int red = 0, green = 0, blue = 0, alpha = 0;

            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int ix = x + kx;
                    int iy = y + ky;

                    if (ix >= 0 && ix < width && iy >= 0 && iy < height) {
                        red += imageIn->pixels[iy][ix].red * boxFilter[ky + 1][kx + 1];
                        green += imageIn->pixels[iy][ix].green * boxFilter[ky + 1][kx + 1];
                        blue += imageIn->pixels[iy][ix].blue * boxFilter[ky + 1][kx + 1];
                        alpha += imageIn->pixels[iy][ix].alpha * boxFilter[ky + 1][kx + 1];
                    }
                }
            }

            imageOut->pixels[y][x].red = red / sum;
            imageOut->pixels[y][x].green = green / sum;
            imageOut->pixels[y][x].blue = blue / sum;
            imageOut->pixels[y][x].alpha = alpha / sum;
        }
    }
}


void *filterThreadWorker(void *args) {
    FilterArgs *filterArgs = (FilterArgs*)args;
    BMP_Image *imageIn = filterArgs->imageIn;
    BMP_Image *imageOut = filterArgs->imageOut;
    int startRow = filterArgs->startRow;
    int endRow = filterArgs->endRow;
    int width = filterArgs->width;
    int height = filterArgs->height;
    int (*boxFilter)[3] = filterArgs->boxFilter;
    
    int sum = 9;

    for (int y = startRow; y < endRow; y++) {
        for (int x = 0; x < width; x++) {
            int red = 0, green = 0, blue = 0;

            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int ix = x + kx;
                    int iy = y + ky;

                    if (ix >= 0 && ix < width && iy >= 0 && iy < height) {
                        red += imageIn->pixels[iy][ix].red * boxFilter[ky + 1][kx + 1];
                        green += imageIn->pixels[iy][ix].green * boxFilter[ky + 1][kx + 1];
                        blue += imageIn->pixels[iy][ix].blue * boxFilter[ky + 1][kx + 1];
                    }
                }
            }

            imageOut->pixels[y][x].red = red / sum;
            imageOut->pixels[y][x].green = green / sum;
            imageOut->pixels[y][x].blue = blue / sum;
        }
    }
    return NULL;
}

void applyParallel(BMP_Image *imageIn, BMP_Image *imageOut, int boxFilter[3][3], int numThreads) {
    int width = imageIn->header.width_px;
    int height = imageIn->norm_height;
    pthread_t threads[numThreads];
    FilterArgs args[numThreads];
    int rowsPerThread = height / numThreads;

    for (int i = 0; i < numThreads; i++) {
        args[i].imageIn = imageIn;
        args[i].imageOut = imageOut;
        args[i].startRow = i * rowsPerThread;
        args[i].endRow = (i == numThreads - 1) ? height : (i + 1) * rowsPerThread;
        args[i].width = width;
        args[i].height = height;
        args[i].boxFilter = boxFilter;
        pthread_create(&threads[i], NULL, filterThreadWorker, &args[i]);
    }

    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }
}
