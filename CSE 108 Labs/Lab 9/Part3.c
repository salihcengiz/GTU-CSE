#include <stdio.h>
#include <string.h>

void print_special_substrings_helper(const char *str, int start, int end) {
    if (end == strlen(str)) {
        return;
    }
    else if (start > end) {
        print_special_substrings_helper(str, 0, end + 1);
    }
    else if (str[start] == str[end]) {
        printf("%.*s\n", end - start + 1, (str + start));
        print_special_substrings_helper(str, start + 1, end);
    }
    else {
        print_special_substrings_helper(str, start + 1, end);
    }
}

void print_special_substrings(const char *str) {
    print_special_substrings_helper(str, 0, 0);
}

int main() {
    print_special_substrings("abcab");
    return 0;
}








