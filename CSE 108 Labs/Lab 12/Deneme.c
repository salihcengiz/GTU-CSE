#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MAX_WORD 20

void initialize_grid(char ***grid, int *row, int *column, char *filename);
void print_grid(char **grid, int row, int column);
void convert_to_uppercase(char **grid, char *word, int start_row, int start_col, int direction_row, int direction_column);
int search_direction(char **grid, int rows, int cols, char *word, int start_row, int start_col, int dir_row, int dir_col);
void search_grid(char **grid, int row, int column, char *word);

int main() {
    char *filename = malloc(sizeof(char) * MAX_WORD);
    char *search = malloc(sizeof(char) * MAX_WORD);
    char **grid;
    int rows, cols;

    printf("Enter the name of the file: ");
    scanf("%s", filename);
    initialize_grid(&grid, &rows, &cols, filename);
    print_grid(grid, rows, cols);
    do {
        printf("Enter the word (\"exit\" to end the program): ");
        scanf("%s", search);
        if (strcmp(search, "exit") == 0) {
            break;
        }
        search_grid(grid, rows, cols, search);
        print_grid(grid, rows, cols);
    } while (1);

    for (int i = 0; i < rows; i++) {
        free(grid[i]);
    }
    free(grid);
    free(filename);
    free(search);

    return 0;
}

void initialize_grid(char ***grid, int *row, int *column, char *filename) {
    int i,j;
    char ch;
    FILE *ptr = fopen(filename, "r");
    if (ptr == NULL) {
        printf("Error: Could not open file %s\n", filename);
        return;
    }
    fscanf(ptr, "%d %d", row, column);
    *grid = malloc(*row * sizeof(char *));
    for (i = 0; i < *row; i++) {
        (*grid)[i] = malloc(*column * sizeof(char));
    }

    for (i = 0; i < *row; i++) {
        for (j = 0; j < *column; j++) {
            ch = fgetc(ptr);
            if (ch == '\n') {
                 ch = fgetc(ptr);
            }
            (*grid)[i][j] = tolower(ch);
        }
    }

    fclose(ptr);
}

void print_grid(char **grid, int row, int column) {
    int i,j;
    for (i = 0; i < row; i++) {
        for (j = 0; j < column; j++) {
            printf("%c ", (grid[i][j]));
        }
        printf("\n");
    }
}

int search_direction(char **grid, int rows, int cols, char *word, int start_row, int start_col, int dir_row, int dir_col) {
    int i,pattern_row,pattern_column;
    int len = strlen(word);
    for (i = 0; i < len; i++) {
        pattern_row = start_row + i * dir_row;
        pattern_column = start_col + i * dir_col;
        if (pattern_row < 0 || pattern_row >= rows || pattern_column < 0 || pattern_column >= cols || tolower(grid[pattern_row][pattern_column]) != word[i]) {
            return 0;
        }
    }
    return 1;
}

void convert_to_uppercase(char **grid, char *word, int start_row, int start_col, int direction_row, int direction_column) {
    int i, pattern_row, pattern_column;
    int len = strlen(word);
    for (i = 0; i < len; i++) {
        pattern_row = start_row + i * direction_row;
        pattern_column = start_col + i * direction_column;
        grid[pattern_row][pattern_column] = toupper(grid[pattern_row][pattern_column]);
    }
}

void search_grid(char **grid, int row, int column, char *word) {
    int i,j,k;
    int directions[8][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    for (i = 0; i < row; i++) {
        for (j = 0; j < column; j++) {
            for (k = 0; k < 8; k++) {
                if (search_direction(grid, row, column, word, i, j, directions[k][0], directions[k][1])) {
                    convert_to_uppercase(grid, word, i, j, directions[k][0], directions[k][1]);
                    printf("WORD FOUND: %s\n", word);
                    return;
                }
            }
        }
    }
    printf("WORD NOT FOUND: %s\n", word);
}