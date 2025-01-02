#include <stdio.h>

int countLetter(char *str, char ch) {

    int i =0;
    int count = 0;

    while (str[i] != '\0') {
        if (str[i] == ch) {
            count++;
        }
        i++;
    }
    return count;
}

void mostCountedLetter(char str[]){

int i ,j, k;

char alphabet[26] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','r','s','t','u','v','y','z','w','q','x'};  

int count[26];

for(i=0; i <25; i++){

    count[i] = 0;

}

i =0;

while(str[i] != '\0'){

    for(j=0; j <25; j++){

         if(str[i] == alphabet[j])
            count[j]++;
        
    }
    
    i++;
}

for(i=0; i <25; i++){

    printf("%d",count[i]);

    }

}

int main() {

    int i;

    char str[] = "salihin arabasi var";

    mostCountedLetter(str);

    return 0;
}


