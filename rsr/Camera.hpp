#pragma once

#include "Geom.hpp"

struct Camera {
   Float3 eye;
   float b1;
   Float3 center;
   float b2;
   Float3 up;
   float b3;

   Float3 dir;
   float b4;

   Matrix perspective;
};
