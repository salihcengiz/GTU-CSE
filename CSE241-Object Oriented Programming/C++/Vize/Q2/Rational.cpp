#include "Rational.h"

Rational::Rational(int nom, int den) : nominator(nom), denominator(den) {
    if (den == 0)
        exit(-1);
    simplify();
}

Rational::Rational(const Rational &r) : nominator(r.nominator), denominator(r.denominator) {simplify();}

Rational::~Rational() {}

Rational& Rational::operator=(const Rational &r) {
    if (this != &r) {
        nominator = r.nominator;
        denominator = r.denominator;
    }
    return *this;
}

Rational Rational::operator+(const Rational &r) const {
    int newNom = nominator * r.denominator + r.nominator * denominator;
    int newDen = denominator * r.denominator;
    return Rational(newNom, newDen);
}

Rational Rational::operator-(const Rational &r) const {
    int newNom = nominator * r.denominator - r.nominator * denominator;
    int newDen = denominator * r.denominator;
    return Rational(newNom, newDen);
}

Rational Rational::operator/(const Rational &r) const {
    return Rational(nominator * r.denominator, denominator * r.nominator);
}

Rational Rational::operator*(const Rational &r) const {
    return Rational(nominator * r.nominator, denominator * r.denominator);
}

bool Rational::operator==(const Rational &r) const {
    return nominator == r.nominator && denominator == r.denominator;
}

bool Rational::operator!=(const Rational &r) const {
    return !(*this == r);
}

bool Rational::operator>(const Rational &r) const {
    return nominator * r.denominator > r.nominator * denominator;
}

bool Rational::operator<(const Rational &r) const {
    return nominator * r.denominator < r.nominator * denominator;
}

void Rational::setRational(int a, int b) {
    if (b == 0) {
        exit(-1);
    }
    nominator = a;
    denominator = b;
    simplify();
}

void Rational::setNom(int nom) {
    nominator = nom;
    simplify();
}

void Rational::setDenom(int den) {
    if (den == 0) {
        exit(-1);
    }
    denominator = den;
    simplify();
}

int Rational::getNom() const {
    return nominator;
}

int Rational::getDenom() const {
    return denominator;
}

void Rational::print() {
   cout<<"The rational number is "<<getNom()<<"/"<<getDenom()<<endl;
}

double Rational::doubleForm() {
   return static_cast<double>(getNom())/getDenom();
}

float Rational::floatForm() {
   return static_cast<float>(getNom())/getDenom();
}

void Rational::simplify() {
   int gcd;
   for (int i = 1; i < getNom() || i < getDenom(); i++){
      if(getNom() % i == 0 && getDenom() % i == 0)
         gcd = i;
   }  
   setNom(getNom() / gcd);
   setDenom(getDenom() / gcd);
}
