#include <stdio.h>

int main(){

int dizi1 [3][2];
int dizi2 [3][2];
int dizi3 [2][3][2];

int i, j, k;

    printf("Dizi 1 Degerlerini Giriniz: \n");

    for(i =0 ; i<3; i++){
        

        for(j =0; j<2; j++){
            
            scanf("%d", &dizi1[i][j]);

    }

    }

    printf("Dizi 2 Degerlerini Giriniz: \n");

    for(i =0 ; i<3; i++){
        

        for(j =0; j<2; j++){
            
            scanf("%d", &dizi2[i][j]);

        }

    }

    if (k == 0){

        for(i = 0 ; i<3; i++){
        

             for(j = 0; j<2; j++){
        
                 dizi3[0][i][j] = dizi1[i][j];
        
            }


        }

    }

    k++;

    if (k == 1){

        for(i = 0 ; i<3; i++){
        

             for(j = 0; j<2; j++){
        
                 dizi3[1][i][j] = dizi2[i][j];
        
            }


        }
        
    }

    for(k = 0; k<2; k++){

        for(i = 0 ; i<3; i++){
        

        for(j = 0; j<2; j++){
        
            printf("dizi3[%d][%d][%d] = %d\n",k,i,j,dizi3[k][i][j]);
        
    }

    }
        
    }

    return 0;
}