#include <stdio.h>

int count_char(char *str, char ch) {
    if (*str == '\0') {
        return 0;
    }
    int count = count_char(str + 1, ch);
    if (*str == ch) {
        count++;
    }
    return count;
}

int main() {
    char str[] = "salihin arabasi var";
    char ch = 'i';
    int count = count_char(str, ch);
    printf("The character '%c' appears %d times in the string \"%s\".\n", ch, count, str);
    return 0;
}
