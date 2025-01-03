#include <iostream>
#include "rational.h"

using namespace std;

void rationalNumbers::setNom(int nominator){
   nom = nominator;
}
void rationalNumbers::setDenom(int denominator){
   denom = denominator;
}

void rationalNumbers::setRational(int a, int b) {
   setNom(a);
   
   if(b != 0)
      setDenom(b);
   else 
      setDenom(1);   
}

int rationalNumbers::getNom(){
   return nom;
}

int rationalNumbers::getDenom(){
   return denom;
}

void rationalNumbers::print() {
   cout<<"The rational number is "<<getNom()<<"/"<<getDenom()<<endl;
}

double rationalNumbers::doubleForm() { // Noktadan sonra kaç basamak return edeceği nasıl belirlenir bak.
   return static_cast<double>(getNom())/getDenom();
}

float rationalNumbers::floatForm() { // Noktadan sonra kaç basamak return edeceği nasıl belirlenir bak.
   return static_cast<float>(getNom())/getDenom();
}

void rationalNumbers::simplify() {
   int gcd;
   for (int i = 1; i < getNom() || i < getDenom(); i++){
      if(getNom() % i == 0 && getDenom() % i == 0)
         gcd = i;
   }  
   setNom(getNom() / gcd);
   setDenom(getDenom() / gcd);
}

rationalNumbers::rationalNumbers(int nom, int den) {
   setNom(nom);
   if(den != 0)
      setDenom(den);
   else 
      setDenom(1);   
}