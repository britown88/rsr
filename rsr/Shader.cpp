#include "GL/glew.h"

#include "Shader.hpp"
#include "Model.hpp"

#include <string>
#include <vector>
#include <unordered_map>


class Shader {
   std::string m_filename;
   int m_params;
   bool m_built;
   GLuint m_handle;
   std::unordered_map<StringView, Uniform> m_uniforms;

   char *readFullFile(const char *path, long *fsize) {
      char *string;
      FILE *f = fopen(path, "rb");

      if (!f) {
         return nullptr;
      }

      fseek(f, 0, SEEK_END);
      *fsize = ftell(f);
      fseek(f, 0, SEEK_SET);

      string = (char*)malloc(*fsize + 1);
      fread(string, *fsize, 1, f);
      fclose(f);

      string[*fsize] = 0;

      return string;
   }
   unsigned int compile(std::vector<const char*> &lines, int type) {
      unsigned int handle = glCreateShader(type);
      if (handle) {

         int compileStatus;
         const GLchar **source = lines.data();
         glShaderSource(handle, lines.size(), lines.data(), nullptr);
         glCompileShader(handle);

         glGetShaderiv(handle, GL_COMPILE_STATUS, &compileStatus);
         if (!compileStatus) {

            int infoLen = 0;
            glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &infoLen);
            std::vector<GLchar> infoLog(infoLen);
            glGetShaderInfoLog(handle, infoLen, &infoLen, &infoLog[0]);
            std::string err = infoLog.data();

            return 0;
         }
      }

      return handle;
   }
   unsigned int link(unsigned int vertex, unsigned int fragment) {
      int handle = glCreateProgram();
      if (handle)
      {
         int linkStatus;
         if (vertex == -1 || fragment == -1) {
            return 0;
         }

         glBindAttribLocation(handle, (GLuint)VertexAttribute::Pos2, "aPosition2");
         glBindAttribLocation(handle, (GLuint)VertexAttribute::Pos3, "aPosition3");
         glBindAttribLocation(handle, (GLuint)VertexAttribute::Tex2, "aTexCoords");
         glBindAttribLocation(handle, (GLuint)VertexAttribute::Col4, "aColor");
         glBindAttribLocation(handle, (GLuint)VertexAttribute::Norm3,"aNormal");

         glAttachShader(handle, vertex);
         glAttachShader(handle, fragment);
         glLinkProgram(handle);

         glGetProgramiv(handle, GL_LINK_STATUS, &linkStatus);
         if (!linkStatus) {
            GLsizei log_length = 0;
            GLchar message[1024];
            glGetProgramInfoLog(handle, 1024, &log_length, message);

            GLsizei srclen = 0;
            GLchar vsrc[10240], fsrc[10240];
            glGetShaderSource(vertex, 10240, &srclen, vsrc);
            glGetShaderSource(fragment, 10240, &srclen, fsrc);

            return 0;
         }
      }
      return handle;
   }
   void build() {
      long fSize = 0;
      auto file = readFullFile(m_filename.c_str(), &fSize);

      if (!file) {
         return;
      }

      std::string Version = "#version 420\n";
      std::string VertexOption = "#define VERTEX\n";
      std::string FragmentOption = "#define FRAGMENT\n";
      std::string DiffuseTextureOption = "#define DIFFUSE_TEXTURE\n";
      std::string Position2DOption = "#define POSITION_2D\n";

      std::vector<const char*> vertShader, fragShader;

      vertShader.push_back(Version.c_str());
      vertShader.push_back(VertexOption.c_str());
      if (m_params&DiffuseTexture) {
         vertShader.push_back(DiffuseTextureOption.c_str());
      }
      if (m_params&Position2D) {
         vertShader.push_back(Position2DOption.c_str());
      }
      vertShader.push_back(file);
      auto vert = compile(vertShader, GL_VERTEX_SHADER);

      fragShader.push_back(Version.c_str());
      fragShader.push_back(FragmentOption.c_str());
      if (m_params&DiffuseTexture) {
         fragShader.push_back(DiffuseTextureOption.c_str());
      }
      fragShader.push_back(file);
      auto frag = compile(fragShader, GL_FRAGMENT_SHADER);

      auto handle = link(vert, frag);
      free(file);

      if (handle) {
         m_handle = handle;
         m_built = true;
      }
   }

public:
   Shader(const char *file, int params) :m_filename(file), m_params(params), m_built(false) {}
   ~Shader() {
   }

   void setActive() {
      if (!m_built) {
         build();
      }
      auto err = glGetError();
      glUseProgram(m_handle);
      err = glGetError();

   }
   Uniform getUniform(const StringView name) {
      if (!m_built) {
         return (Uniform)-1;
      }

      auto found = m_uniforms.find(name);
      if (found == m_uniforms.end()) {
         Uniform u = glGetUniformLocation(m_handle, (const char*)name);
         m_uniforms.insert(std::make_pair(name, u));
         return u;
      }

      return found->second;
   }
   void setFloat2(Uniform u, Float2 const &value) {
      glUniform2fv(u, 1, (float*)&value);
   }
   void setMatrix(Uniform u, Matrix const &value) {
      glUniformMatrix4fv(u, 1, false, (float*)&value);
   }
   void setColor(Uniform u, ColorRGBAf const &value) {
      glUniform4fv(u, 1, (float*)&value);
   }
   void setTexSlot(Uniform u, TextureSlot const &value) {
      glUniform1i(u, value);
   }
};

Shader *ShaderManager::create(const char *file, int params) {
   return new Shader(file, params);
}

void ShaderManager::destroy(Shader *self) {
   delete self;
}

void ShaderManager::setActive(Shader *self) { self->setActive(); }
Uniform ShaderManager::getUniform(Shader *self, StringView name) { return self->getUniform(name); }
void ShaderManager::setFloat2(Shader *self, Uniform u, Float2 const &value) { self->setFloat2(u, value); }
void ShaderManager::setMatrix(Shader *self, Uniform u, Matrix const &value) { self->setMatrix(u, value); }
void ShaderManager::setColor(Shader *self, Uniform u, ColorRGBAf const &value) { self->setColor(u, value); }
void ShaderManager::setTextureSlot(Shader *self, Uniform u, TextureSlot const &value) { self->setTexSlot(u, value); }
