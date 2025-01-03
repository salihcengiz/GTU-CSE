#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib>
#define SIZE_OF_ARRAY 100

using namespace std;

void printArray (int array[], int size) {
    for (int i = 0; i < size; i++){
        cout<<array[i];
    }
}

void fillArray (int array[], int size, int &enteredNumber) {

    int number;
    int i = 0;

    cout<<"Please enter a number(for quit enter 241)\n";

    do{

    cin>>number;

    if (number != 241){
        array[i] = number;
        enteredNumber++;
        i++;
    }

    }while(enteredNumber < size && number != 241);
}

void swapValues(int &n, int &m) {
    int temp = n;
    n = m;
    m = temp;
}

int findIndexOfSmallest(int arr[], int size) {
    int indexOfMinElement = 0;
    int minElement = arr[0];
    for(int i = 1; i < size - 1; ++i){
        if(arr[i] < minElement)
            indexOfMinElement = i;
    }
    return indexOfMinElement;
}

void sortWithSelectionSort(int array[], int size) {
    for(int i = 0; i < size - 1; ++i){
        swapValues(array[i], array[findIndexOfSmallest(array, size)]);
    }
}

int main() {

    int enteredSize = 0;
    int arr[SIZE_OF_ARRAY];

    fillArray(arr, SIZE_OF_ARRAY, enteredSize);
    printArray(arr, enteredSize);

    return 0;
}