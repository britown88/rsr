#pragma once

#include "Geom.hpp"
#include "Texture.hpp"

#include <stdint.h>
#include <string>
#include <vector>

class CubeMap;

class CubeMapManager {

public:
   static CubeMap *create(std::vector<std::string> const &faceFiles);
   static void destroy(CubeMap *self);

   static void bind(CubeMap *self, TextureSlot slot);
};
