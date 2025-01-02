#include <stdio.h>

int isPrime(int n){

    int i;

    for(i = 2 ; i < n ;i++){

        if(n % i == 0){

            return 1;
        }
        return 0;
    }
}   

int countPrimes(int arr[], int n){

int count = 0 ;
int i;

for(i =0 ; i<n ; i++){

    if(isPrime(arr[i]) == 0)
        count++;
}   

    return count;
}

int main(){

    int array[] = {2,3,4,5,6,7,8,9,11,13};

    countPrimes(array,10);

    printf("Number of prime numbers in this array is %d",countPrimes(array,10));

    return 0;
}