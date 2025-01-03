#include <iostream>
#include <string>

// Function Templates: Bir fonksiyonun farklı türlerde çalışmasını sağlamak için kullanılır

void swapValues(int& variable1, int& variable2) { // swapValues sadece int inputlar için çalışıyor
    int temp;
    temp = variable1;
    variable1 = variable2;
    variable2 = temp;
}

//template <class T> de aynı işlevi görür
template<typename T>
void swapValues(T& variable1, T& variable2) { // swapValues artık her type için çalışıyor
    T temp;
    temp = variable1;
    variable1 = variable2;
    variable2 = temp;
}

int main() {
    int integer1 = 1, integer2 = 2;
    std::cout << "Original integer values are " << integer1 << " " << integer2 << std::endl;
    swapValues(integer1, integer2);
    std::cout << "Swapped integer values are " << integer1 << " " << integer2 << std::endl;
    double double1 = 5.32, double2 = 8.73;
    std::cout << "Original double values are: " << double1 << " " << double2 << std::endl;
    swapValues(double1,double2);
    std::cout << "Swapped double values are: " << double1 << " " << double2 << std::endl;
    char symbol1 = 'A', symbol2 = 'B';
    std::cout << "Original character values are: " << symbol1 << " " << symbol2 << std::endl;
    swapValues(symbol1, symbol2);
    std::cout << "Swapped character values are: " << symbol1 << " " << symbol2 << std::endl;
    std::string s1 = "Hello", s2 = "World"; //Fonksiyon type'lar dışında sınıflar için de çalışıyor
    std::cout << "Original string values are: " << s1 << " " << s2 << std::endl;
    swapValues(s1, s2);
    std::cout << "Swapped string values are: " << s1 << " " << s2 << std::endl;
    return 0;
}