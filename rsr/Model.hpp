#pragma once

#include "Geom.hpp"
#include "Color.hpp"

#include <vector>


class Model;

enum ModelOpts : unsigned int {
   IncludeColor = 1 << 0,
   IncludeTexture = 1 << 1,
   IncludeNormals = 1 << 2
};

struct ModelVertices {
   std::vector<Float3> positions;
   std::vector<Float2> textures;
   std::vector<Float3> normals;
   std::vector<ColorRGBAf> colors;

   std::vector<int> positionIndices;
   std::vector<int> textureIndices;
   std::vector<int> normalIndices;

   static std::vector<ModelVertices> fromOBJ(const char *file);

   ModelVertices &calculateNormals();
   ModelVertices &expandIndices();
   Model *createModel(int modelOptions);
};

enum class VertexAttribute : unsigned int {
   Pos2 = 0,
   Pos3,
   Tex2,
   Col4,
   Norm3,
   COUNT
};

int vertexAttributeByteSize(VertexAttribute attr);

#pragma region Vertex objects

#define FVF_ATTRS(...) \
   static std::vector<VertexAttribute> &attrs() { \
      static std::vector<VertexAttribute> out; \
      if (out.empty()) { \
         out = {__VA_ARGS__}; \
      } \
      return out; \
   }

class FVF_Pos2_Tex2_Col4 {
public:
   FVF_ATTRS( VertexAttribute::Pos2, VertexAttribute::Tex2, VertexAttribute::Col4 )
   Float2 pos2, tex2; ColorRGBAf col4;
};

class FVF_Pos2_Col4 {
public:
   FVF_ATTRS(VertexAttribute::Pos2, VertexAttribute::Col4)
   Float2 pos2; ColorRGBAf col4;
};

class FVF_Pos3 {
public:
   FVF_ATTRS(VertexAttribute::Pos3)
   Float3 pos3;
};

class FVF_Pos3_Col4 {
public:
   FVF_ATTRS( VertexAttribute::Pos3, VertexAttribute::Col4 )
   Float3 pos3; ColorRGBAf col4;
};

class FVF_Pos3_Tex2_Col4 {
public:
   FVF_ATTRS(VertexAttribute::Pos3, VertexAttribute::Tex2, VertexAttribute::Col4)
   Float3 pos3; Float2 tex2; ColorRGBAf col4;
};

class FVF_Pos3_Tex2 {
public:
   FVF_ATTRS(VertexAttribute::Pos3, VertexAttribute::Tex2)
   Float3 pos3; Float2 tex2;
};

class FVF_Pos3_Norm3_Tex2_Col4 {
public:
   FVF_ATTRS(VertexAttribute::Pos3, VertexAttribute::Norm3, VertexAttribute::Tex2, VertexAttribute::Col4)
   Float3 pos3, norm3; Float2 tex2; ColorRGBAf col4;
};

class FVF_Pos3_Norm3_Tex2 {
public:
   FVF_ATTRS(VertexAttribute::Pos3, VertexAttribute::Norm3, VertexAttribute::Tex2)
   Float3 pos3, norm3; Float2 tex2;
};

class FVF_Pos3_Norm3_Col4 {
public:
   FVF_ATTRS(VertexAttribute::Pos3, VertexAttribute::Norm3, VertexAttribute::Col4)
   Float3 pos3, norm3; ColorRGBAf col4;
};

class FVF_Pos3_Norm3 {
public:
   FVF_ATTRS(VertexAttribute::Pos3, VertexAttribute::Norm3)
   Float3 pos3, norm3;
};

#pragma endregion

class ModelManager {
   static Model *_create(void *data, size_t size, size_t vCount, VertexAttribute *attrs, int attrCount);
public:
   template<typename FVF>
   static Model *create(FVF *data, size_t vCount) {
      return _create(data, sizeof(FVF), vCount, FVF::attrs().data(), FVF::attrs().size());
   }

   enum RenderType {
      Triangles,
      Lines,
      COUNT
   };

   static void destroy(Model *self);
   static void bind(Model *self);
   static void draw(Model *self, RenderType type = Triangles);
};

