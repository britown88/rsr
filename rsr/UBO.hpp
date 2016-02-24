#pragma once

#include "Geom.hpp"

#include <stdint.h>

class UBO;
typedef uintptr_t UBOSlot;

class UBOManager {
public:
   static UBO *create(size_t size);
   static void destroy(UBO *self);

   static void setMatrix(UBO *self, size_t offset, Matrix const &value);
   static void bind(UBO *self, UBOSlot slot);
};