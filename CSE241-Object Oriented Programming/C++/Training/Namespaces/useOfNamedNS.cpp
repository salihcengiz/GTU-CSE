#include <iostream>

//Bu dosyanın amacı useOfUnnamedNS, useOfNamedNS ve main birlikte çalışacak şekilde anlatım yapmaktır
namespace Cengiz {
    int variableOfNamed = 20;

    void functionOfNamed(){
        std::cout << "Named namespace is accessable." << std::endl;
    }
}   