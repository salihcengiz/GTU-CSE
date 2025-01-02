#include <stdio.h>

    void pascalTriangle(int n){

    if( n % 2 == 1){

        n += 1;
    }

    int i;

    for(i = n; i > 0 ; i--){
    
       
       
    }



    }

    void Diamondl_Triangle_and_Odd_Check (int n){

    if( n % 2 == 0)
    n += 2;
    

    int i, j;

    for (i = 1; i <= n/2; i++) 
    {
        
        for (j = n/2; j > i; j--) 
        {
            printf(" ");
        }

        for (j = 1; j <= i + 1; j++) 
        {
            printf("*");
        }

        printf("\n");
    }

    for (i = n/2; i >= 1; i--) 
    {
        
        for (j = 1; j < (n/2) + 1 - i; j++) 
        {
            printf(" ");
        }

        
        for (j = 1; j <= 2 * i - 1; j++) 
        {
            printf("*");
        }

        printf("\n");
    }

    }

int main() {

    int choice;
    
    int patternSize;

    printf("\nWelcome to the program!\n");
    
    while (1) {

        printf("\nMenu:\n");
        printf("1. Pascal's Triangle\n");
        printf("2. Diamond Pattern and Odd Number Check\n");
        printf("3. Exit\n");
        printf("Enter your choice: \n");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number between 0 and 2.\n");
            
            scanf("%*[^\n]");
            continue;
        }

        switch (choice) {

            case 1:

                printf("Enter the pattern size: \n");
                scanf("%d", &patternSize);
                pascalTriangle(patternSize);
                break;
   
            case 2:

                printf("Enter the pattern size: \n");
                scanf("%d", &patternSize);
                Diamondl_Triangle_and_Odd_Check(patternSize);
                break;

            case 3:

            printf("Exiting program.\n");
                return 0;

            default:
                
                printf("Invalid choice. Please enter a number between 0 and 2.\n"); 
                break;
        }
    }
}
