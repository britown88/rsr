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
   static const ColorRGBAf White =     {  1.0f,  1.0f,  1.0f, 1.0f };
   static const ColorRGBAf Gray =      {  0.5f,  0.5f,  0.5f, 1.0f };
   static const ColorRGBAf DkGray =    { 0.25f, 0.25f, 0.25f, 1.0f };
   static const ColorRGBAf LtGray =    { 0.75f, 0.75f, 0.75f, 1.0f };
   static const ColorRGBAf Black =     {  0.0f,  0.0f,  0.0f, 1.0f };
   static const ColorRGBAf Red =       {  1.0f,  0.0f,  0.0f, 1.0f };
   static const ColorRGBAf DkRed =     {  0.5f,  0.0f,  0.0f, 1.0f };
   static const ColorRGBAf Green =     {  0.0f,  1.0f,  0.0f, 1.0f };
   static const ColorRGBAf DkGreen =   {  0.0f,  0.5f,  0.0f, 1.0f };
   static const ColorRGBAf Blue =      {  0.0f,  0.0f,  1.0f, 1.0f };
   static const ColorRGBAf DkBlue =    {  0.0f,  0.0f,  0.5f, 1.0f };
   static const ColorRGBAf Cyan =      {  0.0f,  1.0f,  1.0f, 1.0f };
   static const ColorRGBAf Yellow =    {  1.0f,  1.0f,  0.0f, 1.0f };
   static const ColorRGBAf Magenta =   {  1.0f,  0.0f,  1.0f, 1.0f };
}

