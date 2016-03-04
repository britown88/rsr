#pragma once

#include "Geom.hpp"
#include "Model.hpp"

#include <vector>

struct TrackPoint {
   Float3 pos;
   float roll, width;
};


Model *createTrackSegment(std::vector<TrackPoint> &pointList, bool wrap);
