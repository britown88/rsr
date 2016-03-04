#pragma once

#include "StringView.hpp"

#include <stdint.h>

#include "Color.hpp"
#include "Geom.hpp"
#include <memory>
#include <string>

struct TextureBuffer {
   std::unique_ptr<ColorRGBA[]> bits;
   Int2 size;

   TextureBuffer() {}

   TextureBuffer(std::unique_ptr<ColorRGBA[]> bits, Int2 size)
      :bits(std::move(bits)), size(size) {}

   TextureBuffer(TextureBuffer && rhs)
      :bits(std::move(rhs.bits)), size(rhs.size) {}
   TextureBuffer &operator=(TextureBuffer && rhs) {
      bits = std::move(rhs.bits);
      size = rhs.size;
      return *this;
   }
};

TextureBuffer loadPng(std::string const& textureFile);

typedef uintptr_t TextureSlot;

enum RepeatType : unsigned int {
   Repeat = 0,
   Clamp
};

enum FilterType : unsigned int {
   Linear = 0,
   Nearest
};

struct TextureRequest {
   RepeatType repeatType;
   FilterType filterType;
   StringView path;

   TextureRequest(StringView path, RepeatType repeat = RepeatType::Clamp, FilterType filter = FilterType::Linear);
   bool operator==(const TextureRequest &rhs)const;
   size_t hash() const;
};

class Texture;

class TextureManager{
public:
   static Texture *get(TextureRequest const &request);
   static void bind(Texture *self, TextureSlot slot);
};


