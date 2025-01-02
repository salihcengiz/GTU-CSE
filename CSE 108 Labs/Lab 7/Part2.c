#include <stdio.h>

int max_array_size = 100;

void setArray(double array[], int size){

    printf("Please Enter Indexes\n");
    int i;

    for(i =0; i<size; i++){

        scanf("%lf",&array[i]);
    }
}

double getArrayElement(double array[], int index){

    return array[index];
}

void printArray(double array[], int size){
    
    int i;

    printf("Updated array elements: ");

    for(i =0; i<size; i++){

        printf("%.2lf ",array[i]);
    }
}

void updateArray(double array[], int size){

    int i;

    for(i=0; i<size; i++){

        if(i%2 == 0){

            array [i] = getArrayElement(array,i)/2;
        }
        else{

            array [i] = getArrayElement(array,i)*2;
        }
    }

    printArray(array,size);
}

int main(){

    int array_size;
    

    do{

    printf("Please Enter An Array Size: \n");
    scanf("%d",&array_size);

    }while(array_size > max_array_size);
 
    double array[array_size];
    setArray(array,array_size);

    updateArray(array,array_size);

    return 0;
}