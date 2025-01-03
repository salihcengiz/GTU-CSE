#ifndef RAT_H
#define RAT_H

class rationalNumbers {
    public:
        rationalNumbers(int nominator = 0, int denominator = 1); //Constructor'u public'e koymayı unutma yoksa default olarak private olur.
        void setRational(int a, int b);
        void setNom(int nom);
        void setDenom(int den);
        int getNom();
        int getDenom();
        void print();
        double doubleForm(); 
        float floatForm();
        void simplify();

    private:
        int nom;
        int denom;
};

#endif