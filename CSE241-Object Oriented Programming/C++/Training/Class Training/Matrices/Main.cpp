#include "Matrix.h"

int main() {

    Matrix2D MyMatrix1(2,2); //2'ye 2 matrisli obje
    MyMatrix1.setElement(0, 0, 1);
    MyMatrix1.setElement(0, 1, 2);
    MyMatrix1.setElement(1, 0, 3);
    MyMatrix1.setElement(1, 1, 4);

    cout << MyMatrix1;

    Matrix2D sum = MyMatrix1 + MyMatrix1;

    cout << sum;

    Matrix2D neg;

    neg = -MyMatrix1;

    cout << neg;

    if (MyMatrix1 == neg)
        cout << "Equal" << endl;

    Matrix2D MyMatrix2;      //1'e 1 matrisli obje
    Matrix2D MyMatrix3(4,5); //4'e 5 matrisli obje
    Matrix2D MyMatrix4(4,5); //4'e 5 matrisli obje
    Matrix2D MyMatrix5(3,2); //3'e 2 matrisli obje

    return 0;
}