#include<stdio.h>

int main()
{
    int input;
    int temp;
    int solution;

    printf("Please Enter A Number: \n");
    scanf("%d",&input);

    temp = input;

    input = (input/7)*7;

    input = input + 7;

    solution = input - temp;

    printf("%d", solution);
    
}