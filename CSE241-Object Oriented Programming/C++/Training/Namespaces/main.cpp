#include <iostream>
#include "useOfNamedNS.cpp"
#include "useOfUnnamedNS.cpp"

//Bu dosyanın amacı useOfUnnamedNS, useOfNamedNS ve main birlikte compile edilince ve ayrı ayrı compile edilip linklenince yaşanacaklar
//üzerinden konuyu kavramaktır

int main() {   
    using namespace Cengiz;
    
    int variableOfGlobalNS = 30; //Global namespace'te variable oluşturduk
    std::cout << variableOfGlobalNS << std::endl; //Global namespace'e daima erişimimiz var

    std::cout << Cengiz::variableOfNamed << ", "; //#include "useOfNamedNS.cpp" yaparsak named namespace'e erişimimiz var
    Cengiz::functionOfNamed();                    //#include "useOfNamedNS.cpp" yaparsak named namespace'e erişimimiz var
    //Eğer useOfNamedNS.cpp dosyasını include etmek yerine önce -c ile object dosyasına dönüştürüp sonra main main ile linkleseydim
    //aynı compilation unit'te olmalarına gerek olmadığı için named namespace yine erişilebilir olacaktı.

    std::cout << variableOfUnnamed << ", ";       //#include "useOfUnnamedNS.cpp" yaparsak named namespace'e erişimimiz var
    functionOfUnnamed();                          //#include "useOfUnnamedNS.cpp" yaparsak named namespace'e erişimimiz var
    //Ancak eğer useOfUnnamedNS.cpp dosyasını include etmek yerine önce -c ile object dosyasına dönüştürüp sonra main ile linkleseydim
    //o zaman aynı compilation unit'te olmadıkları için unnamed namespace erişilebilir olmayacaktı.

    //yani "useOfNamedNS.cpp" ve "useOfUnnamedNS.cpp" include ediliyken g++ main.cpp; ./a.out yeterliyken 
    //"useOfNamedNS.cpp" ve "useOfUnnamedNS.cpp" include edili değilken unnamed namespace dolayısıyla kod çalışmaz


    return 0;
}