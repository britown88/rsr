#include "Window.hpp"
#include "Renderer.hpp"

#include "Model.hpp"
#include "UBO.hpp"

Model *buildTestModel() {

   std::vector<FVF_Pos3_Tex2_Col4> vertices = {
      { { 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f, 1.0f } },
      { { 1.0f, 0.0f, 0.0f },{ 1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } },
      { { 1.0f, 1.0f, 0.0f },{ 1.0f, 1.0f },{ 0.0f, 0.0f, 1.0f, 1.0f } },
      { { 0.0f, 1.0f, 0.0f },{ 0.0f, 1.0f },{ 1.0f, 1.0f, 1.0f, 1.0f } }
   };

   std::vector<int> indices = { 0, 1, 3, 1, 2, 3 };

   return ModelManager::create(
      vertices.data(),
      vertices.size(),
      indices.data(),
      indices.size());
}

void testRender(Renderer &r, Shader *s, Model *m, Texture *t) {

   auto uModel = internString("uModelMatrix");
   auto uColor = internString("uColorTransform");
   auto uTexture = internString("uTexMatrix");
   auto uTextureSlot = internString("uTexture");


   r.viewport({ 0, 0, (int)r.getWidth(), (int)r.getHeight() });
   r.clear({ 0.0f, 0.0f, 0.0f, 1.0f });

   Matrix modelTransform =
      //Matrix::translate2f({ 100.0f, 100.0f }) *
      Matrix::scale3f({ 100.0f, 100.0f, 50.0f });

   //Matrix viewTransform = Matrix::ortho(0.0f, (float)r.getWidth(), (float)r.getHeight(), 0.0f, 1.0f, -1.0f);

   Matrix persp = Matrix::perspective(90.0f,  4.0f / 3.0f, 0.0001f, 100000.0f);
   Matrix lookAt = Matrix::lookAt({ 50.0f, 50.0f, 75.0f }, { 50.0f, 50.0f, 25.0f }, { 0.0f, 1.0, 0.0f });

   Matrix viewTransform = persp * lookAt;

   Matrix texTransform = Matrix::identity();

   ColorRGBAf colorTransform = { 0.0f, 1.0f, 1.0f, 1.0f };

   UBO *ubo = UBOManager::create(sizeof(Matrix));
   r.bindUBO(ubo, 0);

   r.setUBOData(ubo, &viewTransform);

   r.setShader(s);
   r.setMatrix(uModel, modelTransform);
   r.setMatrix(uTexture, texTransform);
   r.setColor(uColor, colorTransform);

   r.bindTexture(t, 0);
   r.setTextureSlot(uTextureSlot, 0);

   r.renderModel(m);

   r.finish();
   r.flush();
}

int main()
{
   Window *win = Window::create(640, 480, "Test!", 0);

   if (!win) {
      return 0;
   }

   Renderer r(win);

   auto m = buildTestModel();
   auto s = ShaderManager::create("assets/shaders.glsl", DiffuseTexture);
   TextureRequest request(internString("assets/00.png"));
   auto t = TextureManager::get(request);

   r.beginRender();

   while (!win->shouldClose()) {
      win->pollEvents();
      testRender(r, s, m, t);
      
   }

   Window::destroy(win);
}