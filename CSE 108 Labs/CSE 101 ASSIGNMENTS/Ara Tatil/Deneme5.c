#include <stdio.h>
#include <stdbool.h>

int main(void) {
  
  int a = 111;
  int b = 222;

  int result;

  bool overflow;

  overflow = __builtin_add_overflow(a, b, &result);

  printf("sum of %d and %d should be: %d\n", a, b, a + b);
  printf("But, C says that %d + %d = %d\n", a, b, result);

  printf("Overflow var mi: %s\n", overflow ? "var" : "yok");

  overflow = __builtin_mul_overflow(a, b, &result);

  printf("product of %d and %d should be: %d\n", a, b, a * b);
  printf("But, C says that %d * %d = %d\n", a, b, result);
  
  printf("Overflow var mi: %s\n", overflow ? "var" : "yok");

  return 0;
}

