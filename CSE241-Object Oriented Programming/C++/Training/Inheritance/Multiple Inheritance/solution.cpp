#include <iostream>

using std::cout;
using std::endl;

class A {
    public:
        virtual void display() { cout << "Class A" << endl; }
};

class B : virtual public A {};
class C : virtual public A {};
class D : public B, public C {};

int main() {
    D obj;
    obj.display(); // Belirsizlik çözülür
    return 0;
}
