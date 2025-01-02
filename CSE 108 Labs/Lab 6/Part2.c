#include <stdio.h>

int fillArray (double arr[], int n){

int i;
double temp;

for(i=0 ; i<n ; i++){

    scanf("%lf",&temp);

    arr[i] = temp;
}

for(i=0 ; i<n ; i++){

    printf("%.2lf\n",arr[i]);

}

}

int main(){

    double array[5];

    fillArray(array,5);

    return 0;
}