#include <stdio.h>

void drawStickman(int position, char armStatus) {
  printf("\n");
  printf("  O\n");
  if (armStatus == 's') {
    printf(" /|\\\n");
  } else {
    printf("  |\n");
  }
  printf("  |\n");
  printf(" / \\\n");

  int i;

  for (i = 0; i < position; ++i) {
    printf(" ");
  }
  printf("^");
  printf("\n");
}

int moveStickman(int position, char move) {

  switch (move) {

    case 'a':
      
      position--;
      break;

    case 'd':
      position++;
      break;

    case 's':
      return -1;
      break;

    default:
      printf("Invalid move. Please use 'a', 'd', or 's'.\n");
  }

  
  return position;
}

int main() {
  int position = 0;
  char armStatus = 'n'; 

  char move;
  do {
    drawStickman(position, armStatus);
    printf("Enter move ('a' for left, 'd' for right, 's' to switch arms, 'e' to exit): ");
    scanf(" %c", &move);

    
    if (move != 's') { 
      position = moveStickman(position, move);
    } else {
      
      armStatus = (armStatus == 'n') ? 's' : 'n';
      if (move == 'd' && armStatus == 's') {
        position++;
      }
    }
  } while (move != 'e');

  printf("Exiting Stickman Animation. Goodbye!\n");
  return 0;
}