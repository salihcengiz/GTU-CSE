#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ROW 10
#define COLUMN 10

typedef struct block {
    char type;
} Block;

int carRow;
int carColumn;
int finishRow;
int finishColumn;

int generateRandom(int limit) {
    
    return rand() % (limit);
}

void initBoard(Block board[ROW][COLUMN]) {
    
    int i, j;

    for (i = 0; i < ROW; i++) {
        for (j = 0; j < COLUMN; j++) {
            board[i][j].type = '.';
        }
    }

    carRow = generateRandom(ROW);
    carColumn = generateRandom(COLUMN);
    finishRow = generateRandom(ROW);
    finishColumn = generateRandom(COLUMN);

    board[carRow][carColumn].type = 'C';
    board[finishRow][finishColumn].type = 'F';
}

void printBoard(Block board[ROW][COLUMN]) {
    
    int i, j;

    for (i = 0; i < ROW; i++) {
        for (j = 0; j < COLUMN; j++) {
            printf("%c ", board[i][j].type);
        }
        printf("\n");
    }
}

void printMoves(char moves[]) {
    
    printf("Moves: %s\n", moves);
}

void moveCar(Block board[ROW][COLUMN], char *moves) {

    int i = 0;
    char direction;

    while(moves[i] != 'r'){

        moves[i] = direction;

        switch (direction) {

            case 'w': 

                carColumn++; 
                break;

            case 's':

                carColumn--; 
                break;

            case 'a':

                carRow--;
                break;

            case 'd':

                carRow++;
                break;

            default:

                printf("Invalid direction!\n");
                return;
	    }

        i++;
    }
	
}

void gamePlay() {
    
    printf("Initial Board:\n");

    Block board[ROW][COLUMN];

    initBoard(board);

    printBoard(board);

    printf("\n");

    do{

    char moves[100];
    printf("Enter the sequence of moves (up, down, left, right): ");
    scanf("%s", moves);

    printMoves(moves);
    moveCar(board,moves);
    printf("Final Board:\n");
    printBoard(board);
    printf("\n");

    }while(carColumn != finishColumn && carRow != finishRow);
}

int main() {

    srand(time(NULL));

    gamePlay();

    return 0;
}