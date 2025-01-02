#include <stdio.h>

void reverseArray(int arr[], int size) {
  int temp;
  for (int i = 0; i < size / 2; i++) {
    temp = arr[i];
    arr[i] = arr[size - 1 - i];
    arr[size - 1 - i] = temp;
  }
}

int sumArray(int arr[], int size) {
  if (size == 0) {
    return 0;
  } else {
    return arr[size - 1] + sumArray(arr, size - 1);
  }
}

int searchArray(int arr[], int size, int element) {
  for (int i = 0; i < size; i++) {
    if (arr[i] == element) {
      return i;
    }
  }
  return -1;
}

void printArray(int arr[], int size) {
  for (int i = 0; i < size; i++) {
    printf("%d ", arr[i]);
  }
  printf("\n");
}

int main() {
  int arr[5];
  int choice;
  int element;
  for (int i = 0; i < 5; i++) {
    printf("Enter element %d: ", i + 1);
    scanf("%d", &arr[i]);
  }
  while (1) {
    printf("\nSelect an operation:\n");
    printf("1. Reverse the array\n");
    printf("2. Sum of array elements\n");
    printf("3. Search for an element\n");
    printf("4. Exit\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);
    switch (choice) {
      case 1:
        reverseArray(arr, 5);
        printf("Reversed array: ");
        printArray(arr, 5);
        break;
      case 2:
        printf("Sum of array elements: %d\n", sumArray(arr, 5));
        break;
      case 3:
        printf("Enter the element to search: ");
        scanf("%d", &element);
        int index = searchArray(arr, 5, element);
        if (index == -1) {
          printf("Element not found\n");
        } else {
          printf("Element found at index %d\n", index);
        }
        break;
      case 4:
        return 0;
      default:
        printf("Invalid choice\n");
    }
  }
}


