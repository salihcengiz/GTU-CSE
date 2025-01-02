#include <stdio.h>

int main(){

    int i1, i2, i3, i4;
    float sonuc1, sonuc2, sonuc3;

    printf("Lutfen 4 integer deger giriniz: \n");
    scanf("%d %d %d %d",&i1, &i2, &i3, &i4);

    sonuc1 = i1*i1 + i3*i4 -i1;

    sonuc2 = i1*3 + i2*5 + i3*7 + i4*9;
    
    sonuc3 = (float)(i1 + i2 + i3 + i4)/(i1 * i2 *i3 * i4);

    printf("Birinci maddenin sonucu: %.2f\n", sonuc1);
    printf("Ikinci maddenin sonucu: %.2f\n", sonuc2);
    printf("Ucuncu maddenin sonucu: %.2f\n", sonuc3);

    return 0;
}