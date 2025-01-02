#include <stdio.h>

void swap_Elements(int *arr, int index1, int index2) {
    
    int swap;
    
    swap = arr[index1];
    arr[index1] = arr[index2];
    arr[index2] = swap;
}

int	main()
{

	int arr[] = { 10 , 25 , 48 , 97 , -4 };
	int size = sizeof(arr) / sizeof(arr[0]);
    
    int index1, index2;

    printf("Enter the indices to swap (separated by space): ");
    scanf("%d %d", &index1, &index2);
    
    swap_Elements(arr, index1, index2);

    printf("Array after swapping elements at indices %d and %d: ", index1, index2);
	
    printf("%d, %d, %d, %d, %d", arr[0], arr[1], arr[2], arr[3], arr[4]); 
	return (0);
}