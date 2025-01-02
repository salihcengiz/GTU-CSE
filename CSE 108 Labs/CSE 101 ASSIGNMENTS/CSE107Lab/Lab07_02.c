#include<stdio.h>

int main()
{
    int array[8]={11,22,33,44,55,66,77,88};
    printf("Original Array: \n");

    for(int i=0 ; i<8 ; i++)
    {
        printf("%d, ", array[i]);

    }
    int temp;
    for( int i=0,j=7 ; i<j ; i++,j-- )
    {
        temp = array[i];
        array[i] = array[j];
        array[j] = temp;
 
    }
}