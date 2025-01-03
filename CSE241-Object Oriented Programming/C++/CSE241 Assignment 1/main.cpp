
#include "AnsiTerminal.h"
#include <iostream>

int main() {
    AnsiTerminal terminal;
    terminal.clearScreen(); // Clear the screen at the beginning

    int row = 10, col = 10;
    terminal.printInvertedAt(row, col, "*"); // Initial cursor position in inverted mode

    char key;
    while (true) {
        key = terminal.getSpecialKey();

        // Clear the previous position
        terminal.printAt(row, col, " ");

        // Update position based on arrow key input
        switch (key) {
            case 'U': row = (row > 1) ? row - 1 : row; break; // Up
            case 'D': row = (row < 24) ? row + 1 : row; break; // Down
            case 'R': col = (col < 80) ? col + 1 : col; break; // Right
            case 'L': col = (col > 1) ? col - 1 : col; break; // Left
            case 'q': return 0; // Quit program if 'q' is pressed
        }

        // Display inverted cursor at new position
        terminal.printInvertedAt(row, col, "*");
    }

    terminal.clearScreen(); // Clear the screen on exit
    return 0;
}
