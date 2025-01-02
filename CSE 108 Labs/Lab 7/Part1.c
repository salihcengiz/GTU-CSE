#include <stdio.h>

int max_array_size = 100;

void insert_ordered(int a[], int *n, int x){

    int temp;
    
    a[*n] = x;

    int i;

    for(i=0; i<*n; i++){

        if(a[i]<x){

            temp = a[i];
            x = a[i];
            a[i+1]= temp;
            
        }
    }
}

void get_max_n(const int a[], int n, int max){

    printf("Top 5 Number: ");
    
    int i;

    for(i=0; i<5; i++){

        printf("%d ",a[i]);
    }
}

int main(){

    int array[max_array_size];
    int i, new_item;

    for(i=0; i<max_array_size ; i++){

    printf("Enter Number:");
    scanf("%d",&new_item);

    insert_ordered(array, &i, new_item);

    get_max_n(array, i, 5);

    }
    
    return 0;
}










/*bir integer arrayi selection sort yöntemi ile sıralayan bir c fonksiyonu yaz */