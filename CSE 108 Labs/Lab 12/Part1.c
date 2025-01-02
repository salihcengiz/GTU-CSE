#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 100

void run_game(char *filename){

    char ch;
    char word[MAX_LINE_LENGTH];
    char map [MAX_LINE_LENGTH][MAX_LINE_LENGTH];
    char line[MAX_LINE_LENGTH];
    int height, width, found;

    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    fscanf(file, "%d %d\n", &height, &width); 

    int i, j, k;

    for (i = 0; i < height; i++) {

        for (j = 0; j < width; j++) {

            if (j == width - 1)
                fscanf(file, "%c\n", &ch);

            else
                fscanf(file, "%c", &ch);


            if (ch >= 'A' && ch <= 'Z') 
                ch = ch + 32;
            
            map[i][j] = ch;
        }
    }
    
    fclose(file);

    do
    {
        printf("Please enter a word\n");
        scanf("%s", &word);

        for (i = 0; i < height; i++) {

            for (j = 0; j < strlen(map[i]); j++) {

                if (map[i][j] == word[0]) {

                    found = 1;

                    for (k = 1; k < strlen(word); k++) {

                        if (map[i][j + k] != word[k]) {
                            
                            found = 0;
                            break;
                        }
                    }
                    if (found) {

                        printf("String found horizontal\n");

                        int i, j;

                        for (i = 0; i < height; i++) {

                            for (j = 0; j < width; j++) {

                                printf("%c", map[i][j]);
                                
                            }
                            printf("\n");
                        }
                    }
                }
            }
        }

        for (j = 0; j < strlen(map[0]); j++) {

            for (i = 0; i < height; i++) {

                if (map[i][j] == word[0]) {

                    found = 1;

                    for (k = 1; k < strlen(word); k++) {

                        if (map[i + k][j] != word[k]) {
                            
                            found = 0;
                            break;
                        }
                    }
                    if (found) {

                        printf("String found vertical\n");

                        for (i = 0; i < height; i++) {

                            for (j = 0; j < width; j++) {

                                printf("%c", map[i][j]);
                                
                            }
                            printf("\n");
                        }
                    }
                }
            }
        }

        for (i = 0; i < height; i++) {

            for (j = 0; j < strlen(map[i]); j++) {

                if (map[i][j] == word[0]) {

                    found = 1;

                    for (k = 1; k < strlen(word); k++) {

                        if (i + k >= height || j + k >= strlen(map[i + k]) || map[i + k][j + k] != word[k]) {

                            found = 0;
                            break;
                        }
                    }
                    if (found) {

                        printf("String found diagonal\n");

                            for (i = 0; i < height; i++) {

                                for (j = 0; j < width; j++) {

                                    printf("%c", map[i][j]);
                                
                            }
                            printf("\n");
                        }
                    }
                }
            }
        }

    } while ((strcmp(word,"exit")) != 0);
    

}

int main() {

    printf("Welcome\n\n");

    char file_name[100];

    printf("Enter the name of file (.txt): ");
    scanf("%s", &file_name);

    run_game(file_name);

    return 0;
}

