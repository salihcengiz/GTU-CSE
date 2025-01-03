#include "Computer.h" //Computer class'ı ve fonksiyonlarına ulaşmak için dosyayı eklemeyi unutma

int main() {

    float clockSpeed;
    int cores, totalCapacity, usedMemory;
    
    cout << "Please enter values for your computer\n";
    cin >> clockSpeed;
    cin >> cores;
    cin >> totalCapacity;
    cin >> usedMemory;

    Computer computer1; //Otomatik initialize [mem(1024, 512), cpu1(4.0, 1), cpu2(4.0, 1) verileriyle]

    cout << computer1 << endl;

    Computer computer2(clockSpeed, cores, totalCapacity, usedMemory);

    computer1.setCPU1(computer2.getCPU1());
    computer1.setCPU2(computer2.getCPU2());
    computer1.setMemory(computer2.getMemory());

    cout << computer1 << endl;

    return 0;
}

