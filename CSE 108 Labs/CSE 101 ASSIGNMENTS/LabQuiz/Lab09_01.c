#include <stdio.h>

int is_palindrome(char *str, int start, int end) {
    if (start == end) {
        return 1;
    }
    if (str[start] != str[end]) {
        return 0;
    }
    return is_palindrome(str, start + 1, end - 1);
}

int main() {
    char str[] = "lavel";
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    if (is_palindrome(str, 0, len - 1)) {
        printf("The string \"%s\" is a palindrome.\n", str);
    } else {
        printf("The string \"%s\" is not a palindrome.\n", str);
    }
    return 0;
}
