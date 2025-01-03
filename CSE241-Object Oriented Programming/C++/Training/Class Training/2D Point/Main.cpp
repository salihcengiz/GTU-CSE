#include "Point2D.h"

int main() {

    Point2D p1;
    cout<< p1;

    Point2D p2(2.3, 7.8);
    cout<< p2;

    {Point2D p3;}

    Point2D p4(4,5);
    cout<< p4;

    return 0;
}