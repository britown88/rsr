#include "CubeMap.hpp"
#include "Texture.hpp"

#include "GL/glew.h"

class CubeMap {
   bool m_built = false;
   size_t m_size = 0;
   uintptr_t m_handle;

   std::vector<std::string> m_faceFiles;

   void build() {
      glGenTextures(1, (GLuint*)&m_handle);
      glActiveTexture(GL_TEXTURE0);

      glBindTexture(GL_TEXTURE_CUBE_MAP, m_handle);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      

      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      
      for (GLuint i = 0; i < m_faceFiles.size(); i++)
      {
         TextureBuffer buff = loadPng(m_faceFiles[i]);

         glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 
            GL_RGBA, buff.size.x, buff.size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, buff.bits.get());
      }
      
      glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

      m_built = true;
   }

public:
   CubeMap(std::vector<std::string> const &faceFiles) :m_faceFiles(faceFiles) { }
   ~CubeMap() {
      if (m_built) {
      }
   }

   void bind(TextureSlot slot) {
      if (!m_built) {
         build();
      }

      glActiveTexture(GL_TEXTURE0 + slot);
      glBindTexture(GL_TEXTURE_CUBE_MAP, m_handle);

   }

};

CubeMap *CubeMapManager::create(std::vector<std::string> const &faceFiles) { return new CubeMap(faceFiles); }
void CubeMapManager::destroy(CubeMap *self) { delete self; }
void CubeMapManager::bind(CubeMap *self, TextureSlot slot) { self->bind(slot); }


