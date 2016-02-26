#pragma once

#include "StringView.hpp"

#include <stdint.h>

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


