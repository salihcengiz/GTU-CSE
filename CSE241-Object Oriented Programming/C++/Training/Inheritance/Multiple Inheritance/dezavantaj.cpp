#include <iostream>

using std::cout;
using std::endl;

class A {
public:
    void display() { cout << "Class A" << endl; }
};

class B : public A {};
class C : public A {};
class D : public B, public C {}; // Elmas problemi

int main() {
    D obj;
    //obj.display(); Hata verir çünkü compiler hangi A'nın display() yöntemi çağrılacak bilemiyor
    return 0;
}
