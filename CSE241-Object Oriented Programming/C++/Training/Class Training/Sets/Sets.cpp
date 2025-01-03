#include "Sets.h"

Set::Set() : elements(new int[10]), capacity(10), size(0) {}

 Set::Set(const Set& other) : elements(new int[other.capacity]), capacity(other.capacity), size(other.size) {
        std::copy(other.elements, other.elements + size, elements);
}

Set::~Set(){
    delete[] elements;
}

Set& Set::operator=(const Set& other){

}

Set Set::operator+(const Set& other){

}

void Set::insert(int element){

}

void Set::erase(int elemen){

}

int Set::getCap() const{

}

int Set::getSize() const{

}

Set Set::operator^(const Set& other){

}

ostream& operator<<(ostream& out, const Set& s){

}