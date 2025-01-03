#include "GTUIntStack.h"
#include <iostream>
using namespace std;

int GTUIntStack::numCreated = 0;

GTUIntStack::GTUIntStack():size(0), capacity(1){
    data = new int [capacity];
    numCreated++;
}

GTUIntStack::GTUIntStack (const GTUIntStack& s){
    size = s.size;
    capacity = s.capacity;
    data = new int [capacity];
    for(int i = 0 ; i < size ; ++i)
        data[i] = s.data[i];
    numCreated++;
}

GTUIntStack::~GTUIntStack(){
    delete [] data;
}

void GTUIntStack::resize(){
    if(size == capacity){
        capacity = 2*capacity;
        int *temp = new int [capacity];
        for(int i = 0 ; i < size ; ++i)
            temp[i] = data[i];
    }
}

int GTUIntStack::size() const{return size;}

int GTUIntStack::getStacksCreated(){return numCreated;}

ostream& operator<<(ostream& out ,const GTUIntStack& s){
    for(int i = 0; i < s.size(); ++i)
        out << s.data[i] << " ";
    return out;
}

bool GTUIntStack::isEmpty() const {
    if(size == 0)   
        return true;
    else
        return false;
}

void GTUIntStack::push(int n){
    if(size == capacity){
        capacity = 2* capacity;
        int *temp = new int[capacity];
        for(int i = 0 ; i < size ; ++i)
            temp[i] = data[i];
        data = temp;
    }
    data[size] = n;
    size++; 
}

void GTUIntStack::pop(){ 
    if(size > 0){
        size--;
    }
    else    
        cout<<"Array is empty"<< endl;
}

