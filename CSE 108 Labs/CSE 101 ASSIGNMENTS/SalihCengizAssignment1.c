#include <stdio.h>  // Include the standard input/output library 

// Function to add two numbers
float add(float a, float b) {
    return a + b;  // Return the sum of a and b
}

// Function to subtract two numbers
float subtract(float a, float b) {
    return a - b;  // Return the difference of a and b
}

// Function to multiply two numbers
float multiply(float a, float b) {
    return a * b;  // Return the product of a and b
}

// Function to divide two numbers
float divide(float a, float b) {
    if(b != 0)  // Check if the divisor is not zero
        return a / b;  // Return the quotient of a and b
    else
        return 0;  // Return 0 if the divisor is zero
}

// Function to calculate the nth power of a number
float power(float a, float b) {
    float result = 1;  // Initialize the result to 1
    for(int i = 0; i < b; i++)  // Loop from 0 to b
        result *= a;  // Multiply the result by a
    return result;  // Return the result
}

// Function to calculate the average of numbers
float average() {
    float sum = 0;  // Initialize the sum to 0
    int count = 0;  // Initialize the count to 0
    float num;  // Declare a variable to store the number
    printf("Enter numbers:\n");  // Prompt the user to enter numbers
    while(scanf("%f", &num) == 1) {  // Loop while the user enters a number
        sum += num;  // Add the number to the sum
        count++;  // Increment the count
    }
    return sum / count;  // Return the average
}

// Function to find the maximum of numbers
float maximum() {
    float max;  // Declare a variable to store the maximum
    float num;  // Declare a variable to store the number
    printf("Enter numbers:\n");  // Prompt the user to enter numbers
    if(scanf("%f", &max) == 1) {  // If the user enters a number
        while(scanf("%f", &num) == 1) {  // Loop while the user enters a number
            if(num > max)  // If the number is greater than the maximum
                max = num;  // Update the maximum
        }
    }
    return max;  // Return the maximum
}

int main() {
    printf("%% WELCOME TO GTU CALCULATOR MACHINE %%\n");  // Print the menu
    printf("%% STUDENT NAME: SALIH CENGIZ %%\n");
    printf("%% PLEASE SELECT FROM THE FOLLOWING %%\n");
    printf("%% MENU : %%\n");
    printf("(1) ADD TWO NUMBERS\n");
    printf("(2) SUBTRACT TWO NUMBERS\n");
    printf("(3) MULTIPLY TWO NUMBERS\n");
    printf("(4) DIVIDE TWO NUMBERS\n");
    printf("(5) TAKE THE NTH POWER OF A NUMBER\n");
    printf("(6) FIND AVERAGE OF NUMBERS INPUTTED\n");
    printf("(7) FIND THE MAXIMUM OF NUMBERS INPUTTED\n");
    printf("(0) EXIT\n");
    printf("PLEASE SELECT:\n");

    int choice;  // Declare a variable to store the choice
    float a, b;  // Declare variables to store the numbers
    while(scanf("%d", &choice) == 1 && choice != 0) {  // Loop while the user enters a choice and the choice is not 0
        switch(choice) {  // Switch on the choice
            case 1:  // Case 1: Add two numbers
                printf("Enter two numbers:\n");  // Prompt the user to enter two numbers
                scanf("%f %f", &a, &b);  // Read the two numbers
                printf("Result: %f\n", add(a, b));  // Print the result of adding the two numbers
                break;  //exit the switch
            case 2:  // Case 2: Subtract two numbers
                printf("Enter two numbers:\n");  // Prompt the user to enter two numbers
                scanf("%f %f", &a, &b);  // Read the two numbers
                printf("Result: %f\n", subtract(a, b));  // Print the result of subtracting the two numbers
                break;  //exit the switch
            case 3:  // Case 3: Multiply two numbers
                printf("Enter two numbers:\n");  // Prompt the user to enter two numbers
                scanf("%f %f", &a, &b);  // Read the two numbers
                printf("Result: %f\n", multiply(a, b));  // Print the result of multiplying the two numbers
                break;  //exit the switch
            case 4:  // Case 4: Divide two numbers
                printf("Enter two numbers:\n");  // Prompt the user to enter two numbers
                scanf("%f %f", &a, &b);  // Read the two numbers
                printf("Result: %f\n", divide(a, b));  // Print the result of dividing the two numbers
                break;  //exit the switch
            case 5:  // Case 5: Take the nth power of a number
                printf("Enter two numbers:\n");  // Prompt the user to enter two numbers
                scanf("%f %f", &a, &b);  // Read the two numbers
                printf("Result: %f\n", power(a, b));  // Print the result of taking the nth power of a number
                break;  //exit the switch
            case 6:  // Case 6: Find the average of numbers inputted
                printf("Result: %f\n", average());  // Print the result of finding the average of numbers inputted
                break;  //exit the switch
            case 7:  // Case 7: Find the maximum of numbers inputted
                printf("Result: %f\n", maximum());  // Print the result of finding the maximum of numbers inputted
                break;  //exit the switch
        }
        printf("PLEASE SELECT:\n");  // Prompt the user to select an option
    }

    return 0;  // Return 0
}
