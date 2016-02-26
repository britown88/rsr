#pragma once

#include "Defs.hpp"

struct ColorRGB {
   byte r, g, b;
};

struct ColorRGBA {
   byte r, g, b, a;
};

struct ColorRGBf {
   float r, g, b;
};

struct ColorRGBAf {
   float r, g, b, a;
};

namespace CommonColors {
   static const ColorRGBAf White = { 1.0f, 1.0f, 1.0f, 1.0f };
}

