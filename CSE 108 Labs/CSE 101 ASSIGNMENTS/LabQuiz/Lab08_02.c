#include <stdio.h>

int ebob(int a, int b) 
{
    if (b == 0)
        return a;
    return ebob(b, a % b);
}

int main() 
{
    int num1, num2;
    printf("Enter two positive integers: ");
    scanf("%d %d", &num1, &num2);
    printf("GCD of %d and %d is %d\n", num1, num2, ebob(num1, num2));
    return 0;
}
