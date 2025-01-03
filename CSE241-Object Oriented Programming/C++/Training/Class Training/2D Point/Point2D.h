#ifndef POINT2D_H
#define POINT2D_H
#include <iostream>

using namespace std;

class Point2D {
    public:
        Point2D(double x = 0.0, double y = 0.0);
        Point2D(const Point2D& other);
        ~Point2D();
        Point2D& operator=(const Point2D& other);
        Point2D& operator!();
        bool operator==(const Point2D& p) const;
        bool operator!=(const Point2D& p)const;
        Point2D& operator++(int i);
        Point2D& operator++();
        Point2D& operator-(const Point2D& p) const;
        void setX(double x) {point_x = x;}
        void setY(double y) {point_y = y;}
        double getX() const {return point_x;}
        double getY() const {return point_y;}
        static int getNumOfAllCreated() {return allCreated;}
        static int getNumOfLivings() {return allLiving;}
        friend ostream& operator<<(ostream& out, const Point2D& p);
    private:
        double point_x;
        double point_y;
        static int allCreated;
        static int allLiving;
};

ostream& operator<<(ostream& out, const Point2D& p);

#endif