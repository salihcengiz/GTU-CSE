#include <stdio.h>
#include <string.h>

int isPalindrome(char *str, int start, int end)
{
    if (start >= end)
        return 1;

    if (str[start] != str[end])
        return 0;

    return isPalindrome(str, start + 1, end - 1);
}

int main()
{
    char str[] = "noon";
    int len = strlen(str);

    if (isPalindrome(str, 0, len - 1))
        printf("It is palindrome");
    else
        printf("It is not palindrome");

    return 0;
}
