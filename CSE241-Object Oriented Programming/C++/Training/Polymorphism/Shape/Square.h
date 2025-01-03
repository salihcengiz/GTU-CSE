#ifndef SQUARE_H
#define SQUARE_H

#include "Rectangle.h"

class Square : public Rectangle {
public:
    Square(double side);
    ~Square() override;
};

#endif // SQUARE_H
