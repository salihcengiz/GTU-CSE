#include <vector>
#include "Shape.h"
#include "Circle.h"
#include "Rectangle.h"
#include <iostream>

using namespace std;

template<class T>
class ShapeContainer {
    public:
        void addShape(const T&);
        double totalArea();
        int NumOfCircles();
    protected:
        vector<T> vec;
};

template<class T>
int ShapeContainer<T>:: NumOfCircles(){
int numCircles;
for(int i=0; i < vec.size(); ++i){
    if(decltype(vec[i]) == decltype(Circle c))
        numCircles++;
} return numCircles;}

template<class T>
double ShapeContainer<T>::totalArea(){
    double total;
    for(int i=0; i < vec.size(); ++i){
    total += vec[i].area();
    }
    return total;
}

template<class T>
void ShapeContainer<T>::addShape(const T&){
    vec.push_back(T);
}

int main() {
    ShapeContainer<Shape> sc;   //Sen ShapeContainer<T> sc; yazdın, mainde T yazamazsın

    Circle c(3.2);
    Rectangle r(4.7,5.8);

    sc.addShape(c);
    sc.addShape(r);

    cout << "Areas of all stored shapes are:"
    << sc.totalArea() << endl;

    cout << "Number of circles, is: "
    << sc.NumOfCircles() << endl;

    return 0;
}