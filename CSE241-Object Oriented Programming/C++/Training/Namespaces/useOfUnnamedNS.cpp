#include <iostream>

//Bu dosyanın amacı useOfUnnamedNS, useOfNamedNS ve main birlikte çalışacak şekilde anlatım yapmaktır
namespace {
    int variableOfUnnamed = 10;

    void functionOfUnnamed(){
        std::cout << "Unnamed namespace is accessable." << std::endl;
    }
}


