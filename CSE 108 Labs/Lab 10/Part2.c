#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

int generateRandom(int limit) {
    
    return rand() % (limit + 1);
}

int findClosestRec(int arr[], int n, int target, int closest) {

    if (n == 0)
        return closest;

    if (abs(arr[n-1] - target) < abs(closest - target))
        closest = arr[n-1];

    return findClosestRec(arr, n-1, target, closest);
}

int main() {

    srand(time(NULL));

    int i;
    int arr[6];

    for(i=0; i<6; i++){

        arr[i] = generateRandom(1000);
    }

    printf("Array: ");

    for(i=0; i<6; i++){

        printf("%d ",arr[i]);
    }

    int target = 30;
    int size = sizeof(arr) / sizeof(arr[0]);

    int closest = findClosestRec(arr, size, target, arr[0]);

    printf("The closest value is: %d\n", closest);

    return 0;
}
