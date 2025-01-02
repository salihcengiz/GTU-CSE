#include <stdio.h>

int fillArray (double arr[], int n){

int i;
double temp;

for(i=0 ; i<n ; i++){

    scanf("%lf",&temp);

    arr[i] = temp;
}

}


int compareArrays(double arr1[], double arr2[], int n, int arr[]){

    int i;
    for(i = 0 ; i<n ; i++){

        if(arr1[i] > arr2[i]){

            arr[i] = 0;
        }
        else if(arr1[i] < arr2[i]){

            arr[i] = 1;
        }
        else{
            
            arr[i] = 2;

        }

    }

    for(i=0 ; i<n ; i++){

    printf("%d\n",arr[i]);

}

}

int main(){

    double arr1[5];
    double arr2[5];
    int result[5];

    fillArray(arr1,5);
    fillArray(arr2,5);
    compareArrays(arr1, arr2, 5, result);

    return 0;
}