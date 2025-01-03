#ifndef PFA_H
#define PFA_H
#include <iostream>
#include <cstdlib>
#include <cassert>

using namespace std;

class PFA { //Bu class aslında kendi vector'ümüzü oluşturmaya benziyor
    public:
        PFA(int cap = 10);  //No parameter and convertion constructor      
        PFA(const PFA& other);  //Copy constructor
        ~PFA(); //Destructor                            
        PFA& operator=(const PFA& other); //Assignment operator
        //DONT' FORGET THE BIG THREE (CONSTRUCTOR, DISTRUCTOR, ASSIGMENT OPERATOR) AS ABOVE 
        double& operator[](int i);
        double operator[](int i) const;
        void addElement(double element);
        double removeElement(); //Exercise
        int getNumberUsed() const {return numberUsed;}
        int getCapacity() const {return capacity;}
        bool isEmpty() const {return numberUsed == 0;}
        void empty() {numberUsed = 0;}
    private:
        double *data;
        int numberUsed;
        int capacity;
};

#endif