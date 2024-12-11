#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#pragma pack(push, 1)
typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BMPFileHeader;

typedef struct {
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BMPInfoHeader;

#pragma pack(pop)

#define TARGET_COLOR_B 127
#define TARGET_COLOR_G 188
#define TARGET_COLOR_R 217

typedef struct {
    uint8_t *pixels;
    int width;
    int startY;
    int endY;
    bool *found;
    pthread_mutex_t *mutex;
} ThreadData;

bool is_L_shape(uint8_t *pixels, int width, int x, int y) {
    int bottomIndex = 0;
    for (int i = 0; i < 8; i++) {
        int leftIndex = ((y + i) * width + x) * 4;
        if (pixels[leftIndex] != TARGET_COLOR_B ||
            pixels[leftIndex + 1] != TARGET_COLOR_G ||
            pixels[leftIndex + 2] != TARGET_COLOR_R) {
            return false;
        }
    }
    for (int j = 0; j < 7; j++) {
        bottomIndex = ((y + 7) * width + (x + j)) * 4;
        if (pixels[bottomIndex] != TARGET_COLOR_B ||
            pixels[bottomIndex + 1] != TARGET_COLOR_G ||
            pixels[bottomIndex + 2] != TARGET_COLOR_R) {
            return false;
        }
    }
    bottomIndex = ((y + 7) * width + (x + 7)) * 4;
    int j = 7 - 2;
    for( int count = 0; count <  pixels[bottomIndex] + pixels[bottomIndex + 2];)
    {
        for (int k = 2; k < 8; k++) {
            int charMessage = ((y + j) * width + (x + k)) * 4;
            printf("%c%c%c", pixels[charMessage], pixels[charMessage + 1], pixels[charMessage + 2]);
            count = count + 3;
        }
        j = j - 1;
    }
    return true;
}

void *scan_for_L_shape(void *arg) {
    ThreadData *data = (ThreadData *)arg;

    for (int y = data->startY; y < data->endY; y++) {
        for (int x = 0; x < data->width - 7; x++) {
            if (is_L_shape(data->pixels, data->width, x, y)) {
                pthread_mutex_lock(data->mutex);
                *(data->found) = true;
                pthread_mutex_unlock(data->mutex);
                return NULL;
            }
        }
    }
    return NULL;
}

int main() {
    const char *filename = "42_logo.bmp";
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        return 1;
    }
    BMPFileHeader fileHeader;
    fread(&fileHeader, sizeof(BMPFileHeader), 1, file);

    BMPInfoHeader infoHeader;
    fread(&infoHeader, sizeof(BMPInfoHeader), 1, file);
    if (fileHeader.bfType != 0x4D42) {
        printf("Not valid.\n");
        fclose(file);
        return 1;
    }
    if (infoHeader.biBitCount != 32) {
        printf("Only supports 32-bit BMP files.\n");
        fclose(file);
        return 1;
    }
    int rowSize = infoHeader.biWidth * 4;
    uint8_t *pixels = (uint8_t *)malloc(rowSize * abs(infoHeader.biHeight));
    fseek(file, fileHeader.bfOffBits, SEEK_SET);
    fread(pixels, 1, rowSize * abs(infoHeader.biHeight), file);
    fclose(file);
    bool found = false;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    int num_threads = 4;
    pthread_t threads[num_threads];
    ThreadData threadData[num_threads];
    for (int i = 0; i < num_threads; i++) {
        int startY = i * (abs(infoHeader.biHeight) / num_threads);
        int endY = (i + 1) * (abs(infoHeader.biHeight) / num_threads);
        if (i == num_threads - 1)
            endY = abs(infoHeader.biHeight);
        threadData[i] = (ThreadData){
            .pixels = pixels,
            .width = infoHeader.biWidth,
            .startY = startY,
            .endY = endY,
            .found = &found,
            .mutex = &mutex
        };
        pthread_create(&threads[i], NULL, scan_for_L_shape, &threadData[i]);
    }
    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);

    free(pixels);
    return 0;
}
