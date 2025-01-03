#include "CPU.h"    //CPU class'ı ve fonksiyonlarına ulaşmak için dosyayı eklemeyi unutma 
#include <iostream>
#include <cstdlib> //Invalid constructor parameter girilince exit ile programı sonlandırmak için

using namespace std; //cout kullanımı sebebiyle

//Constuctor'a invalid değerler gönderilmediğini kontrol etmek için test fonksiyonu
void CPU::testConstructorCPU(){
    if(clockSpeed <= 0) {
        cout << "Invalid clock speed value\n";
        exit(1);
    }

    if(cores <= 0) {
        cout << "Invalid cores value\n";
        exit(1);
    }
}

//Constuctor implement ederken bu syntax ile implement et, diğer syntax'a göre daha verimli
CPU::CPU(float clockSpeedV, int coresV) : clockSpeed(clockSpeedV), cores(coresV) { 
    testConstructorCPU(); //Göderilen değerlerin valid olup olmadığını kontrol ediyoruz
}

//No Parameter Constructor: Kullanıcı objeyi bir değerle initialize etmezse obje bu değerlerle initialize olur
CPU::CPU() : clockSpeed(4.0), cores(1) { 
    //Atamayı kendimiz yağtığımız için (no parameter constructor) parametre validity testi yapmamıza gerek yok, burası boş kalabilir
}

//Bu constructor sadece clock speed değerini kullanıcıdan alarak initialize eder(core otomatik initialize edilir)
CPU::CPU(float clockSpeedV) : clockSpeed(clockSpeedV), cores(1) { 
    testConstructorCPU();
}

//Bu constructor sadece core değerini kullanıcıdan alarak initialize eder(clock speed otomatik initialize edilir)
CPU::CPU(int coresV) : clockSpeed(4.0), cores(coresV) { 
    testConstructorCPU();
}

float CPU::getClockSpeed() const {
    return clockSpeed;
}

void CPU::setClockSpeed(float clockSpeedV){
    clockSpeed = clockSpeedV;
}

int CPU::getCores() const {
    return cores;
}
void CPU::setCores(int coresV){
    cores = coresV;
}