#include <stdio.h>

int main() 
{
    int i, j, k;

    for (i = 4; i >= 1; i--) 
    {
        
        for (j = 1; j < 5 - i; j++) 
        {
            printf(" ");
        }

        
        for (j = 1; j <= 2 * i - 1; j++) 
        {
            printf("*");
        }

        printf("\n");
    }

    for (i = 1; i <= 4; i++) 
    {
        
        for (j = 4; j > i; j--) 
        {
            printf(" ");
        }

        for (k = 1; k <= i + 1; k++) 
        {
            printf("*");
        }

        printf("\n");
    }

    return 0;
}