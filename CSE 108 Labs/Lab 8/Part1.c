#include <stdio.h>
#define MAX_COLS 6
#define MAX_ROWS 15
#define MAX_COLS 6

void readFFloat (char *file_name, float output[][MAX_COLS], int *numrows, int *numcols){

    FILE *fp = fopen(file_name, "r");

    fp = fopen(file_name,"r");

    float num;
    *numrows = 0;

    while (fscanf(fp, "%f", &num) == 1){

        if (*numrows < MAX_ROWS) {

            arr[(*numrows)++][5] = num;
        } 
    }

    fclose(fp);
    
}

int main(){

    float arr[MAX_ROWS][MAX_COLS];

    int row = 15;
    int col =2;

    readFloatsFromFile("dosya.txt", arr, &row, &col);

    printf("Dosyadan okunan float sayilar:\n");
    for (int i = 0; i < col; i++) {

         for (int j = 0; j < row; j++) {

        printf("%f\n", arr[i][j]);

        }
    }

    return 0;
}