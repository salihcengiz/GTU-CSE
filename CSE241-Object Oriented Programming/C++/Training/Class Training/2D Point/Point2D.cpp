#include "Point2D.h"

int Point2D::allCreated = 0;
int Point2D::allLiving = 0;


Point2D::Point2D(double x, double y) : point_x(x), point_y(y) {
    allCreated++;
    allLiving++;
}

Point2D::Point2D(const Point2D& other) : point_x(other.getX()), point_y(other.getY()) {
    allCreated++;
    allLiving++;
}

Point2D::~Point2D() {
    allLiving--;
}

Point2D& Point2D::operator=(const Point2D& other) {
    if (this != &other) { 
        point_x = other.getX();
        point_y = other.getY();
    }
    return *this;
}

Point2D& Point2D::operator!() {
    double temp = point_x;
    point_x = point_y;
    point_y = temp;
    return *this;
}

bool Point2D::operator==(const Point2D& p) const {
     return (point_x == p.getX()) && (point_y == p.getY());
}

bool Point2D::operator!=(const Point2D& p)const {
    return !(*this == p);
}

Point2D& Point2D::operator++(int i) { //Post-increment
    Point2D temp(*this); 
    point_x++;
    point_y++;
    return temp;
}

Point2D& Point2D::operator++() { //Pre-increment
    point_x++;
    point_y++;
    return *this;
}

Point2D& Point2D::operator-(const Point2D& p) const {
    Point2D* result = new Point2D(point_x - p.getX(), point_y - p.getY());
    return *result;
}

ostream& operator<<(ostream& out, const Point2D& p){
    out << "x:" << p.getX() << " y:" << p.getY() << endl;
    out << "All objects created so far:" << Point2D::getNumOfAllCreated() << endl;
    out << "All objects living at the time: " << Point2D::getNumOfLivings() << endl;
    return out;
}