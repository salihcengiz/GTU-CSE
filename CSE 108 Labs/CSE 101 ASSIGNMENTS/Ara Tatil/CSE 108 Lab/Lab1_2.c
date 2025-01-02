#include <stdio.h>
#define Pi 3.14

int main(){

    float cap, alan;

    printf("Lutfen bir cap degeri giriniz: \n");
    scanf("%f",&cap);

    alan = 4 * Pi * cap * cap ;

    printf("Dairenin alani: %.2f \n", alan);
    
    return 0;
}