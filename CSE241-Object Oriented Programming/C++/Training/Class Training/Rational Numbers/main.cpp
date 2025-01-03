#include <iostream>
#include "rational.h"

using namespace std;

int main() {

    rationalNumbers r1;

    r1.print();
    r1.setRational(3,6);
    r1.print();
    r1.simplify();
    r1.print();

    cout<<"The double form of rational number is "<<r1.doubleForm()<<endl;
    cout<<"The double form of rational number is "<<r1.floatForm()<<endl;

    return 0;
}

