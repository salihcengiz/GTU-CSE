#ifndef POLYGON_H
#define POLYGON_H

#include "Shape.h"

class Polygon : public Shape {
protected:
    int sides;
public:
    Polygon(int s);
    ~Polygon() override;
};

#endif // POLYGON_H
