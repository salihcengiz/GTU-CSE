#include "Computer.h" //Computer class'ı ve fonksiyonlarına ulaşmak için dosyayı eklemeyi unutma

int Computer::totalUsedMemory = 0; //Static member variable'ı scope dışında initialize etmeyi unutma

//iostream ve namespace std eklememize gerek yok çünkü Computer.h içindeki Memory.h ve CPU.h içerisinde tanımlılar

//Constuctor implement ederken bu syntax ile implement et, diğer syntax'a göre daha verimli
Computer::Computer(float cSpeed, int cores, int totalCap, int usedMem) : mem(totalCap, usedMem), cpu1(cSpeed, cores), cpu2(cSpeed, cores){
    //Burada olduğu gibi scope işaretinden sonra değişken atamaları obje initialize eder gibi yapılabilir
    //Valid değerler gönderildiğini test etmek bizim değil cpu ve memory class'larını yazanların sorumluluğunda
    totalUsedMemory += mem.getTotalCapacity();
}

//No Parameter Constructor: Kullanıcı objeyi bir değerle initialize etmezse obje bu değerlerle initialize olur
Computer::Computer(): mem(1024, 512), cpu1(4.0, 1), cpu2(4.0, 1){
    //Valid değerler gönderildiğini test etmek bizim değil cpu ve memory class'larını yazanların sorumluluğunda
    totalUsedMemory += mem.getTotalCapacity();
}

CPU Computer::getCPU1() const {
    return cpu1;
}

CPU Computer::getCPU2() const {
    return cpu2;
}

Memory Computer::getMemory() const {
    return mem;
}

void Computer::setCPU1(const CPU& c1){
    cpu1 = c1;
}

void Computer::setCPU2(const CPU& c2) {
    cpu2 = c2;
}

void Computer::setMemory(const Memory& memory) {
    mem = memory;
}

int Computer::totalCores() const {
    return cpu1.getCores() + cpu2.getCores();
}

 //Burada fonksiyonun başına static koymaman lazım yoksa bu keyword fonksiyonu sadece bu dosyada ulaşılabilir hale getirir
int Computer::getTotalUsedMemory() { 
    return totalUsedMemory;
}

float Computer::getCPU1sSpeed() const {
    return cpu1.getClockSpeed();
}

int Computer::getCPU1sCores() const {
    return cpu1.getCores();
}

float Computer::getCPU2sSpeed() const {
    return cpu2.getClockSpeed();
}

int Computer::getCPU2sCores() const {
    return cpu2.getCores();
}

int Computer::getTotalCap() const {
    return mem.getTotalCapacity();
}

int Computer::getUsedMemory() const {
    return mem.getUsedMemory();            
}

//Computer class verilerini bastıran over loaded << operatörü 
//decltype(cout)& operator<<(decltype(cout)& out, const Computer& computer){ satırı ile aynı şey (yani cout'un type'ı ostream)
ostream& operator<<(ostream& out, const Computer& computer){
    
    out << "The values of computer1: "<< endl;
    out << "Clock speed for first CPU: "<< computer.getCPU1sSpeed() << endl;
    out << "Cores for first CPU: "<< computer.getCPU1sCores() << endl;
    out << "Clock speed for second CPU: "<< computer.getCPU2sSpeed() << endl;
    out << "Cores for second CPU: "<< computer.getCPU2sCores() << endl;
    out << "Total capacity: "<< computer.getTotalCap() << endl;
    out << "Used memory: "<< computer.getUsedMemory() << endl;
    out << "Total used memory: " << Computer::getTotalUsedMemory() << endl;
    return out;
}