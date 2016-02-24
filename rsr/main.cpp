#include "Window.hpp"
#include "Renderer.hpp"

#include "Model.hpp"

Model *buildTestModel() {

   std::vector<FVF_Pos2_Col4> vertices = {
      { { 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f, 1.0f } },
      { { 1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } },
      { { 1.0f, 1.0f },{ 0.0f, 0.0f, 1.0f, 1.0f } },
      { { 0.0f, 1.0f },{ 1.0f, 1.0f, 1.0f, 1.0f } }
   };

   std::vector<int> indices = { 0, 1, 3, 1, 2, 3 };

   return ModelManager::create(
      vertices.data(),
      vertices.size(),
      indices.data(),
      indices.size());
}

void testRender(Renderer &r, Shader *s, Model *m) {

   auto uView = internString("uViewMatrix");
   auto uModel = internString("uModelMatrix");
   auto uColor = internString("uColorTransform");
   auto uTexture = internString("uTexMatrix");
   auto uTextureSlot = internString("uTexture");


   r.viewport({ 0, 0, (int)r.getWidth(), (int)r.getHeight() });
   r.clear({ 1.0f, 0.0f, 0.0f, 1.0f });

   Matrix modelTransform =
      Matrix::translate({ 100.0f, 100.0f }) *
      Matrix::scale({ 100.0f, 100.0f });

   Matrix viewTransform =
      Matrix::ortho(0.0f, (float)r.getWidth(), (float)r.getHeight(), 0.0f, 1.0f, -1.0f);

   Matrix texTransform = Matrix::identity();

   ColorRGBAf colorTransform = { 1.0f, 1.0f, 1.0f, 1.0f };

   r.setShader(s);
   r.setMatrix(uView, viewTransform);
   r.setMatrix(uModel, modelTransform);
   //r.setMatrix(uTexture, texTransform);
   r.setColor(uColor, colorTransform);

   //r.bindTexture(m_texture, 0);
   //r.setTextureSlot(uTextureSlot, 0);

   r.renderModel(m);

   r.finish();
}

int main()
{
   Window *win = Window::create(640, 480, "Test!", 0);

   if (!win) {
      return 0;
   }

   Renderer r(win);

   auto m = buildTestModel();
   auto s = ShaderManager::create("assets/shaders.glsl", 0);

   r.beginRender();

   while (!win->shouldClose()) {
      win->pollEvents();
      testRender(r, s, m);
      r.flush();
   }

   Window::destroy(win);
}