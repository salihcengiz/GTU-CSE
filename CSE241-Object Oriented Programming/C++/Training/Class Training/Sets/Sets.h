#ifndef SETS_H
#define SETS_H
#include <iostream>

using namespace std;

class Set {
    public:
        Set();
        Set(const Set& other);
        ~Set();
        Set& operator=(const Set& other);
        Set operator+(const Set& other); //Referans return etmek yanlış
        Set operator^(const Set& other); //Referans return etmek yanlış
        void insert(int element);
        void erase(int element);
        bool find(int element) const;
        int getCap() const;
        int getSize() const;
        friend ostream& operator<<(ostream& out, const Set& s);
    private:
        int *elements;
        int size;
        int capacity;
        void resize(); //GPT önerisi
};

ostream& operator<<(ostream& out, const Set& s);

#endif