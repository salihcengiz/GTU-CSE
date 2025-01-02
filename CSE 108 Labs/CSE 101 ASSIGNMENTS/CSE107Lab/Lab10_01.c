#include <stdio.h>
#include <string.h>

int main(){

  char* str = "Salih kitap aldi";
  char* keyword = "kitap";
  int len = strlen(str);
  int keylen = strlen(keyword);
  int i = 0;
  while (i < len) {
    int j = 0;
    while (j < keylen && i + j < len && str[i + j] == keyword[j]) {
      j++;
    }
    if (j == keylen) {
      printf(keyword);
      i += j;
    } else {
      printf("-");
      i++;
    }
  }
}