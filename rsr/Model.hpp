#pragma once

#include "Geom.hpp"
#include "Color.hpp"

#include <vector>


class Model;

enum class VertexAttribute : unsigned int {
   Pos2 = 0,
   Pos3,
   Tex2,
   Col4,
   COUNT
};

class FVF_Pos2_Tex2_Col4 {
public:
   static std::vector<VertexAttribute> &attrs() {
      static std::vector<VertexAttribute> out;
      if (out.empty()) {
         out = {  VertexAttribute::Pos2,
                  VertexAttribute::Tex2,
                  VertexAttribute::Col4 };
      }
      return out;
   }

   Float2 pos2, tex2;
   ColorRGBAf col4;
};

class FVF_Pos2_Col4 {
public:
   static std::vector<VertexAttribute> &attrs() {
      static std::vector<VertexAttribute> out;
      if (out.empty()) {
         out = {  VertexAttribute::Pos2,
                  VertexAttribute::Col4 };
      }
      return out;
   }

   Float2 pos2;
   ColorRGBAf col4;
};

class ModelManager {
   static Model *_create(void *data, size_t size, size_t vCount, int *indices, int iCount, VertexAttribute *attrs, int attrCount);
public:
   template<typename FVF>
   static Model *create(FVF *data, size_t vCount, int *indices, size_t iCount) {
      return _create(data, sizeof(FVF), vCount, indices, iCount, FVF::attrs().data(), FVF::attrs().size());
   }

   static void destroy(Model *self);
   static void bind(Model *self);
   static void draw(Model *self);

};

