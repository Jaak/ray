#include "geometry.h"

#include <ostream>


std::ostream& operator<<(std::ostream& os, const Vector& v) {
    os << "Vector {" << v.x << "," << v.y << "," << v.z << "}";
    return os;
}
     
std::ostream& operator<<(std::ostream& os, const Point& p) {
    os << "Point {" << p.x << "," << p.y << "," << p.z << "}";
    return os;
}
  
std::ostream& operator<<(std::ostream& os, const Colour& c) {
    os << "Colour {" << c.x << "," << c.y << "," << c.z << "}";
    return os;
}
