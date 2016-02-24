#pragma once

#include "Geom.hpp"
#include "Color.hpp"
#include "StringView.hpp"

#include <stdint.h>

class Shader;
typedef uintptr_t Uniform;

enum ShaderParams : int {
   DiffuseTexture = 1 << 0
};

class ShaderManager {
public:
   static Shader *create(const char *file, int params);
   static void destroy(Shader *self);

   static void setActive(Shader *self);

   static Uniform getUniform(Shader *self, StringView name);
   static void setFloat2(Shader *self, Uniform u, Float2 const &value);
   static void setMatrix(Shader *self, Uniform u, Matrix const &value);
   static void setColor(Shader *self, Uniform u, ColorRGBAf const &value);
   //static void setTextureSlot(Shader *self, Uniform u, TextureSlot const &slot);
};
