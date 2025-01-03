#include <iostream>

#include "Person.h"
#include "Student.h"

// Person ve Student class'ları sadece GTUSpace namespace'inde geçerli, bu namespace'i kullanmadan Person class'ına ulaşılamaz
using namespace GTUSpace;

int main() {
    
    Student Ali("Ali", 2.5, "Computer Engineer");

    std::cout << Ali.getName() << std::endl;
    std::cout << Ali.getMajor() << std::endl;
    std::cout << Ali.getGPA() << std::endl;
    Ali.setName("Veli");
    std::cout << Ali.getName() << std::endl;

    return 0;
}