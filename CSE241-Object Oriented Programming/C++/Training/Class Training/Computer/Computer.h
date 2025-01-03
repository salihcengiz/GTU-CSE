#ifndef COMPUTER_H
#define COMPUTER_H

#include "Memory.h" //Memory class'ı ve fonksiyonlarına ulaşmak için dosyayı eklemeyi unutma 
#include "CPU.h"    //CPU class'ı ve fonksiyonlarına ulaşmak için dosyayı eklemeyi unutma
#include <iostream>
#include <cstdlib> //Invalid constructor parameter girilince exit ile programı sonlandırmak için

using namespace std; //cout kullanımı sebebiyle

class Computer {
    public:
        Computer();
        Computer(float, int, int, int); //Prototypingde parametre adı yazmaya gerek yok
        CPU getCPU1() const;        //Ana objeyi korumak için const kullanımını unutma
        CPU getCPU2() const;        //Ana objeyi korumak için const kullanımını unutma
        Memory getMemory() const;   //Ana objeyi korumak için const kullanımını unutma
        int totalCores() const;     //Ana objeyi korumak için const kullanımını unutma
        float getCPU1sSpeed() const;  //Ana objeyi korumak için const kullanımını unutma
        int getCPU1sCores() const;  //Ana objeyi korumak için const kullanımını unutma
        float getCPU2sSpeed() const;  //Ana objeyi korumak için const kullanımını unutma
        int getCPU2sCores() const;  //Ana objeyi korumak için const kullanımını unutma
        int getTotalCap() const;    //Ana objeyi korumak için const kullanımını unutma
        int getUsedMemory() const;  //Ana objeyi korumak için const kullanımını unutma
        void setCPU1(const CPU&);       //Gönderilen parametreyi korumak için const kullanımını unutma
        void setCPU2(const CPU&);       //Gönderilen parametreyi korumak için const kullanımını unutma
        void setMemory(const Memory&);  //Gönderilen parametreyi korumak için const kullanımını unutma
        static int getTotalUsedMemory();
    private:
        CPU cpu1;
        CPU cpu2;
        Memory mem;
        static int totalUsedMemory;
};

ostream& operator<<(ostream& out, const Computer& computer);

#endif