#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

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

bool is_L_shape(uint8_t *pixels, int width, int x, int y) {
    int leftIndex = 0;
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
    int count = 0;
    int j = 7 - 2;
    while(count <  pixels[bottomIndex] + pixels[bottomIndex + 2])
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

int main() {
    const char *filename = "example.bmp";

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
        printf("Not a valid BMP file.\n");
        fclose(file);
        return 1;
    }

    if (infoHeader.biBitCount != 32) {
        printf("Only 32-bit BMP files are supported.\n");
        fclose(file);
        return 1;
    }

    int rowSize = infoHeader.biWidth * 4;

    uint8_t *pixels = (uint8_t *)malloc(rowSize * abs(infoHeader.biHeight));
    fseek(file, fileHeader.bfOffBits, SEEK_SET);
    fread(pixels, 1, rowSize * abs(infoHeader.biHeight), file);
    fclose(file);

    bool found = false;
    for (int y = 0; y < abs(infoHeader.biHeight) - 7; y++) {
        for (int x = 0; x < infoHeader.biWidth - 7; x++) {
            if (is_L_shape(pixels, infoHeader.biWidth, x, y)) {
                printf("Found L-shape at (%d, %d)\n", x, y);
                found = true;
                break;
            }
        }
        if (found) break;
    }

    if (!found) {
        printf("L-shape not found.\n");
    }

    free(pixels);
    return 0;
}
