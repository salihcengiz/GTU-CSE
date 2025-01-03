#include "Memory.h" //Memory class'ı ve fonksiyonlarına ulaşmak için dosyayı eklemeyi unutma
#include <iostream>
#include <cstdlib> //Invalid constructor parameter girilince exit ile programı sonlandırmak için

using namespace std; //cout kullanımı sebebiyle

//Constuctor'a invalid değerler gönderilmediğini kontrol etmek için test fonksiyonu
void Memory::testConstructorMemory(){
    if (totalCapacity <= 0) {
        cout << "Invalid total capacity value\n";
        exit(1);
    }

    if (usedMemory <= 0) {
        cout << "Invalid used memory value\n";
        exit(1);
    }

    if (usedMemory > totalCapacity) {
        cout << "The used memory can't be greater than the total capacity.\n";
        exit(1);        
    }
}

//Constuctor implement ederken bu syntax ile implement et, diğer syntax'a göre daha verimli
Memory::Memory(int totalCap, int usedMem) : totalCapacity(totalCap), usedMemory(usedMem) {
    testConstructorMemory(); //Göderilen değerlerin valid olup olmadığını kontrol ediyoruz
}

//No Parameter Constructor: Kullanıcı objeyi bir değerle initialize etmezse obje bu değerlerle initialize olur
Memory::Memory() : totalCapacity(1024), usedMemory(512) {
    //Atamayı kendimiz yağtığımız için (no parameter constructor) parametre validity testi yapmamıza gerek yok, burası boş kalabilir
}

/*
//Diğer tek parametreli constructor ile overload olamadığı için kullanım dışı
//Bu constructor sadece total capacity değerini kullanıcıdan alarak initialize eder(used memory otomatik initialize edilir)
Memory::Memory(int totalCap) : totalCapacity(totalCap), usedMemory(512) {
    testConstuctor();
}

//Diğer tek parametreli constructor ile overload olamadığı için kullanım dışı
//Bu constructor sadece used memory değerini kullanıcıdan alarak initialize eder(total capacity otomatik initialize edilir)
Memory::Memory(int usedMem) : totalCapacity(1024), usedMemory(usedMem) {
    testConstuctor();
}
*/

int Memory::getTotalCapacity() const {
    return totalCapacity;
}

void Memory::setTotalCapacity(int totalCap){
    totalCapacity = totalCap;
}

int Memory::getUsedMemory() const {
    return usedMemory;
}
void Memory::setUsedMemory(int usedMem){
    usedMemory = usedMem;
} 