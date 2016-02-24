#pragma once

#include "Geom.hpp"

struct Camera {
   Float3 eye;
   Float3 center;
   Float3 up;

   Matrix perspective;
};
