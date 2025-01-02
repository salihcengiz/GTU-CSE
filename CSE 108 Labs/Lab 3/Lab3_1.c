#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int getValidGuess() {
  int guess;
  do {
    printf("Error!Enter your guess (between 1 and 1000): ");
    if (scanf("%d", &guess) != 1) {
      
      printf("Invalid input. Please enter a number.\n");
      scanf("%d",&guess);
    }
  } while (guess <= 1 || guess >= 1000);
  return guess;
}

int main() {
  
  srand(time(0));

  int target = rand() % 1000 + 1;

  int counter = 0;

  int guess;

  
  printf("Welcome to the Guessing Game!\n");

  do {
    counter++;
    guess = getValidGuess();

    if (guess < target) {
      printf("Your guess is less than the target.\n");
    } else if (guess > target) {
      printf("Your guess is greater than the target.\n");
    }
  } while (guess != target);

  printf("Congratulations! You guessed the number! Total guesses made: %d\n", counter);

  return 0;
}
