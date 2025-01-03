#include <iostream>

using std::cout;
using std::endl;

class Engine {
    public:
        void start() { cout << "Engine started" << endl; }
};

class Wheels {
    public:
        void rotate() { cout << "Wheels are rotating" << endl; }
};

class Car : public Engine, public Wheels {
    public:
        void drive() {
            start();  // Engine'den miras
            rotate(); // Wheels'den miras
            cout << "Car is driving" << endl;
        }
};

int main() {
    Car myCar;
    myCar.drive();
    return 0;
}
