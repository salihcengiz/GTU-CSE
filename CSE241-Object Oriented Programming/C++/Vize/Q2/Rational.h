#ifndef RATIONAL_H
#define RATIONAL_H
#include <iostream>
#include <cstdlib>
#include <vector>

using namespace std;

class Rational {
    public:
        Rational(int nom = 0, int den = 1); //No parameter and convertion constructor    
        Rational(const Rational &r); //Copy constructor
        ~Rational(); //Destructor
        Rational& operator=(const Rational &r); //Assignment operator
        Rational operator+(const Rational &r) const;
        Rational operator-(const Rational &r) const;
        Rational operator/(const Rational &r) const;
        Rational operator*(const Rational &r) const;
        bool operator==(const Rational &r) const;
        bool operator!=(const Rational &r) const;
        bool operator>(const Rational &r) const;
        bool operator<(const Rational &r) const;
        void setRational(int a, int b);
        void setNom(int nom);
        void setDenom(int den);
        int getNom() const;
        int getDenom() const;
        void print();
        double doubleForm(); 
        float floatForm();
        void simplify();

    private:
        int nominator;
        int denominator;
};

#endif