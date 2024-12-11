#include <stdio.h>
#include <stdlib.h>

void printGrid(int *grid, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d", grid[i * cols + j]);
        }
        printf("\n");
    }
}

int main() {
    int rows = 0;
    int cols = 8;

    FILE *file = fopen("text.txt", "r");
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }

    char ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') {
            rows++;
        }
    }

    if (rows < 8) {
        rows = 8;
    }
    rewind(file);

    int *grid = (int *)malloc(rows * cols * sizeof(int));
    if (grid == NULL) {
        perror("Failed to allocate memory");
        fclose(file);
        return 1;
    }

    for (int i = rows - 1; i >= 0; i--) {
        for (int j = 0; j < cols; j++) {
            if (fscanf(file, "%1d", &grid[i * cols + j]) != 1) {
                grid[i * cols + j] = 0;
            }
        }
    }

    fclose(file);

    //the work begins from here
    

    printGrid(grid, rows, cols);

    free(grid);

    return 0;
}
