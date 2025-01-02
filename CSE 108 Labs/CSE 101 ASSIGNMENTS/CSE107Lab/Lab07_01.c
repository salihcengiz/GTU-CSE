#include<stdio.h>

int main()
{
    int array[5];
    int sum = 0;

    printf("Enter 5 intigers: \n");
    for( int i= 0 ; i<5 ; i++ )
    {
        scanf("%d", &array[i]);

    }
    for( int i = 0 ; i<5 ; i++ )
    {
        sum = sum + array[i];

    }
    printf("Sum: %d\n", sum);
    printf("Average: %.2f", (float) sum/5);
}