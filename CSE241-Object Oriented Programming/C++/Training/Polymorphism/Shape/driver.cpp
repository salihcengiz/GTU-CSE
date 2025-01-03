#include <iostream>
#include <vector>
#include "Shape.h"
#include "Circle.h"
#include "Rectangle.h"

using namespace std;

bool isLarger(const Shape* s1, const Shape* s2) {
    return s1->area() > s2->area();
}

Shape* chooseMax(const vector<Shape*>& shapes) {
    Shape* maxShape = shapes[0];
    for (const auto& shape : shapes) {
        if (shape->area() > maxShape->area()) {
            maxShape = shape;
        }
    }
    return maxShape;
}

int main() {
    Circle c1(1.0);
    Rectangle r1(10.0, 20.0);

    cout << "Circle area: " << c1.area() << endl;
    cout << "Rectangle area: " << r1.area() << endl;

    cout << "Circle is larger than Rectangle: " << (isLarger(&c1, &r1) ? "Yes" : "No") << endl;

    vector<Shape*> shapes = {&c1, &r1};
    cout << "Largest shape area: " << chooseMax(shapes)->area() << endl;

    return 0;
}
