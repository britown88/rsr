#pragma once

#include <memory>

#include "Window.hpp"
#include "Model.hpp"
#include "Shader.hpp"
//#include "Texture.hpp"

#include "Color.hpp"
#include "Geom.hpp"
#include "StringView.hpp"
#include "UBO.hpp"
#include "CubeMap.hpp"


class Renderer {
   class Impl;
   std::unique_ptr<Impl> pImpl;

   void _setUBOData(UBO *ubo, size_t offset, size_t size, void *data);
public:
   Renderer(Window *wnd);
   ~Renderer();

   size_t getWidth() const;
   size_t getHeight() const;

   //utility
   void finish();
   void flush() const;
   void beginRender() const;

   //render functions
   void clear(ColorRGBAf const &c);
   void viewport(Recti const &r);

   void enableDepth(bool enabled);
   void enableAlphaBlending(bool enabled);
   void enableWireframe(bool enabled);

   void setShader(Shader *s);
   void setFloat2(StringView u, Float2 const &value);
   void setMatrix(StringView u, Matrix const &value);
   void setColor(StringView u, ColorRGBAf const &value);

   void setTextureSlot(StringView u, TextureSlot const &value);
   void bindTexture(Texture *t, TextureSlot slot);

   template<typename T>
   void setUBOData(UBO *ubo, T &value) {
      _setUBOData(ubo, 0, sizeof(T), &value);
   }
   void bindUBO(UBO *ubo, UBOSlot slot);
   void bindCubeMap(CubeMap *cm, TextureSlot slot);

   void renderModel(Model *m);

};
