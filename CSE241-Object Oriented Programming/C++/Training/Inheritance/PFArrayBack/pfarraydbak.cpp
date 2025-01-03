//This is the file pfarraydbak.cpp.
//This is the implementation of the class PFArrayDBak.
//The interface for the class PFArrayDBak is in the file pfarraydbak.h.

#include "pfarraydbak.h"
#include <iostream>

using std::cout;

PFArrayDBak::PFArrayDBak( ) : PFArrayD( ), usedB(0) {
    b = new double[capacity];
}
/* Hocanın implementasyonu:
PFArrayDBak::PFArrayDBak( ) : PFArrayD( ), usedB(0), b(nullptr) {}
*/

PFArrayDBak::PFArrayDBak(int capacityValue) : PFArrayD(capacityValue), usedB(0) {
    b = new double[capacity];
}
/* Hocanın implementasyonu:
PFArrayDBak::PFArrayDBak(int capacityValue) : PFArrayD(capacityValue), usedB(0), b(nullptr) {}
*/

PFArrayDBak::PFArrayDBak(const PFArrayDBak& oldObject) : PFArrayD(oldObject), usedB(0) {
    b = new double[capacity];
    usedB = oldObject.usedB;
    for (int i = 0; i < usedB; i++)
        b[i] = oldObject.b[i];
}
/* Hocanın implementasyonu:
PFArrayDBak::PFArrayDBak(const PFArrayDBak& oldObject) : PFArrayD(oldObject), usedB(oldObject.usedB), b(nullptr) {
    if (o.b != nullptr){
        b = new double[usedB];
        for (int i = 0; i < usedB; i++)
            b[i] = oldObject.b[i];
    }
}
*/

PFArrayDBak::~PFArrayDBak() {
    delete [] b; // The destructor for the base class PFArrayD is called automatically, and it performs delete [ ] a;
}
/* Hocanın implementasyonu:
PFArrayDBak::~PFArrayDBak() {
    if (b != nullptr)
        delete [] b;
}
*/

void PFArrayDBak::backup( ) { //Note that b is a copy of the array a. We do not want to use b = a; (shallow copy)
    usedB = used;
    for (int i = 0; i < usedB; i++)
        b[i] = a[i];
}
/* Hocanın implementasyonu:                         //Base class operator kullanarak
void PFArrayDBak::backup( ) {
    if (b != nullptr)
        delete [] b;
    usedB = getNumberUsed();
    if (usedB != 0)
        b = new double[getNumberUsed()];
    else    
        b = nullptr;

    for (int i = 0; i < used; i++)
        b[i] = PFArrayD::operator[](i);             //Alternatif syntax
}
*/

void PFArrayDBak::restore( ) {
    used = usedB;
    for (int i = 0; i < used; i++)
        a[i] = b[i];
}
/* Hocanın implementasyonu:                         //addElement() kullanarak
void PFArrayDBak::restore( ) {
    emptyArray();
    for (int i = 0; i < used; ++i)
        addElement(b[i]);                           //Alternatif syntax
}
*/

PFArrayDBak& PFArrayDBak::operator=(const PFArrayDBak& rightSide) {
    int oldCapacity = capacity;
    PFArrayD::operator =(rightSide); // Use a call to the base class assignment operator in order to assign to the base class member variables.
    if (oldCapacity != rightSide.capacity) {
        delete [] b;
        b = new double[rightSide.capacity];
    }
    usedB = rightSide.usedB;
    for (int i = 0; i < usedB; i++)
        b[i] = rightSide.b[i];
    return *this;
}
/* Hocanın implementasyonu:
PFArrayDBak& PFArrayDBak::operator=(const PFArrayDBak& rightSide) {
    if (this == &rightSide)
        return *this;

    PFArrayD::operator =(rightSide);
    usedB = rightSide.usedB;
    if (b != nullptr)
        delete [] b;

    if (rightSide.b != nullptr){
        b = new double[rightSide.usedB];
        for (int i = 0; i < usedB; i++)
            b[i] = rightSide.b[i];
    }
}
*/

