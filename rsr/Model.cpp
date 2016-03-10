#include "GL/glew.h"

#include "Model.hpp"
#include "Defs.hpp"

#include <memory>

int vertexAttributeByteSize(VertexAttribute attr) {
   switch (attr) {
   case VertexAttribute::Tex2:
   case VertexAttribute::Pos2:
      return sizeof(Float2);
      break;
   case VertexAttribute::Pos3:
   case VertexAttribute::Norm3:
      return sizeof(Float3);
      break;
   case VertexAttribute::Col4:
      return sizeof(ColorRGBAf);
      break;
   }

   return 0;
}


class Model {
   std::unique_ptr<byte[]> m_data;
   std::unique_ptr<int[]> m_indices;
   size_t m_vertexSize;
   size_t m_vertexCount;
   size_t m_indexCount;

   bool m_built;

   std::vector<VertexAttribute> m_attrs;

   GLuint m_vboHandle, m_iboHandle;

   void build() {
      glGenBuffers(1, (GLuint*)&m_vboHandle);
      glBindBuffer(GL_ARRAY_BUFFER, m_vboHandle);
      glBufferData(GL_ARRAY_BUFFER, m_vertexSize * m_vertexCount, m_data.get(), GL_STATIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      glGenBuffers(1, (GLuint*)&m_iboHandle);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_iboHandle);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * m_indexCount, m_indices.get(), GL_STATIC_DRAW);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

      m_built = true;
   }

public:
   Model(void *data, size_t size, size_t vCount, int *indices, int iCount, VertexAttribute *attrs, int attrCount)
      : m_vertexSize(size),
      m_vertexCount(vCount),
      m_indexCount(iCount),
      m_attrs(attrs, attrs + attrCount),
      m_data(new byte[size * vCount]),
      m_indices(new int[iCount]),
      m_built(false) {

      //NEVERFORGET the night brandon spent 2 hours debugging empty data
      memcpy(m_data.get(), data, size * vCount);
      memcpy(m_indices.get(), indices, iCount * sizeof(int));
   }

   ~Model() {
   }

   void bind() {
      if (!m_built) {
         build();
      }

      glBindBuffer(GL_ARRAY_BUFFER, m_vboHandle);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_iboHandle);

      //clear current attribs
      for (unsigned int i = 0; i < (unsigned int)VertexAttribute::COUNT; ++i) {
         glDisableVertexAttribArray(i);
      }

      int totalOffset = 0;
      for (auto && attr : m_attrs) {
         glEnableVertexAttribArray((unsigned int)attr);
         int count = 0;
         int offset = totalOffset;

         totalOffset += vertexAttributeByteSize(attr);

         switch (attr) {
         case VertexAttribute::Tex2:
         case VertexAttribute::Pos2:
            count = 2;
            break;
         case VertexAttribute::Pos3:
         case VertexAttribute::Norm3:
            count = 3;
            break;
         case VertexAttribute::Col4:
            count = 4;
            break;
         }

         glVertexAttribPointer((unsigned int)attr,
            count, GL_FLOAT, GL_FALSE, m_vertexSize, (void*)offset);
      }
   }

   void render(ModelManager::RenderType type) {
      static GLuint map[ModelManager::RenderType::COUNT];
      static bool mapInit = false;
      if (!mapInit) {
         mapInit = true;
         map[ModelManager::Triangles] = GL_TRIANGLES;
         map[ModelManager::Lines] = GL_LINES;
      }

      glDrawElements(map[type], m_indexCount, GL_UNSIGNED_INT, 0);
   }
};

Model *ModelManager::_create(void *data, size_t size, size_t vCount, int *indices, int iCount, VertexAttribute *attrs, int attrCount) {
   return new Model(data, size, vCount, indices, iCount, attrs, attrCount);
}

void ModelManager::destroy(Model *self) {
   delete self;
}

void ModelManager::bind(Model *self) { self->bind(); }
void ModelManager::draw(Model *self, RenderType type) { self->render(type); }
