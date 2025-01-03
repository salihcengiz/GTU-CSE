#ifndef PFA_H
#define PFA_H

#include <vector>
#include <stdexcept>

class PFA {
public:
    PFA(int cap = 10);
    PFA(const PFA& o); // Copy constructor - vector kullandığımız için gerek yok (kendi copy constructor'ı yazılmış)
    ~PFA();  // Destructor - vector kullandığımız için gerek yok (kendi copy destructor'ı yazılmış)
    
    PFA& operator=(const PFA& o); // Assignment operator - vector kullandığımız için gerek yok (kendi assignment operator'ı yazılmış)
    
    int getNumUsed() const { return data.size(); }
    int getCap() const { return data.capacity(); }
    
    void setCap(int newCap); // Resize the vector to a new capacity
    
    void addElement(double d);
    double removeElement(); 
    bool isEmpty() const;
    void empty();
    
    double operator[](int i) const; // Const subscript operator
    double& operator[](int i);      // Non-const subscript operator

private:
    std::vector<double> data;
};

#endif
