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


class Renderer {
   class Impl;
   std::unique_ptr<Impl> pImpl;
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

   void setShader(Shader *s);
   void setFloat2(StringView u, Float2 const &value);
   void setMatrix(StringView u, Matrix const &value);
   void setColor(StringView u, ColorRGBAf const &value);

   void setTextureSlot(StringView u, TextureSlot const &value);
   void bindTexture(Texture *t, TextureSlot slot);

   void setUBOMatrix(UBO *ubo, size_t offset, Matrix const &value);
   void bindUBO(UBO *ubo, UBOSlot slot);

   void renderModel(Model *m);

};
