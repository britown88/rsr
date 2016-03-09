#include "Geom.hpp"

#include <vector>

struct Face {
   Float3 vertices[3];
   Plane p;
};

ConvexHull quickHull(std::vector<Float3> &pointCloud) {
   return ConvexHull();
}