#include <stdio.h>


int main()
{
    int arr[128];
    int n = 0, i = 0, min = 0, max = 0;
    
    printf("Enter the number of elements of the array: ");
    scanf("%d", &n);
    
    for(i = 0; i < n; i++) {
        printf("arr[%d] = ", i);
        scanf("%d", &arr[i]);
    }
    
    min = arr[0];
    max = arr[0];
    
    for(i = 1; i < n; i++) {
        if(arr[i] < min) {
            min = arr[i];
        } else if (arr[i] > max) {
            max = arr[i];
        }
    }
    
    printf("Minimum number in the array: %d.\n", min);
    printf("Maximum number in the array: %d.\n", max);
    return 0;
}