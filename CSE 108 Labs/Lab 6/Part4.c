#include <stdio.h>

int searchQuery(char arr1[], char arr2[]){

    int i = 0 ,j = 0;
    int r;

    while(arr1[i] != '\0'){
        
        while(arr2[j] != '\0'){

            if (arr1[i + j] != arr2[j])
                break;

            j++;
        }

        if (j == 5)
            return i; 

        i++;
    }

        return 0;
}

int main(){

    char array1[] = "Ali eve gitti ve yemek yedi";
    char array2[] = "yemek";

    searchQuery(array1,array2);

    if(searchQuery != 0 ){

        printf("yemek is contained in target at position %d.",searchQuery);

    }

    else{

        printf("Query not found.\n");

    }

    return 0;
}