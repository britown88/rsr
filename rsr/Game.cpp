#include "Game.hpp"
#include "Camera.hpp"

class Game::Impl {
   Renderer &m_renderer;
   Window *m_window;

   Model *m_testModel;
   Shader *m_testShader;
   Texture *m_testTexture;
   UBO *m_testUBO;

   Camera m_camera;

   Model *buildTestModel() {
      std::vector<FVF_Pos3_Tex2_Col4> vertices = {
         { { 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f, 1.0f } },
         { { 1.0f, 0.0f, 0.0f },{ 1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } },
         { { 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f },{ 0.0f, 0.0f, 1.0f, 1.0f } },
         { { 0.0f, 1.0f, 1.0f },{ 0.0f, 1.0f },{ 1.0f, 1.0f, 1.0f, 1.0f } }
      };

      std::vector<int> indices = { 0, 1, 3, 1, 2, 3 };

      return ModelManager::create(
         vertices.data(),
         vertices.size(),
         indices.data(),
         indices.size());
   }
   void buildCamera() {
      float aspectRatio = (float)m_window->getWidth() / (float)m_window->getHeight();

      m_camera.perspective = Matrix::perspective(
         140.0f, //fov                                       
         aspectRatio, //aspect ratio
         0.01f, // zNear
         FLT_MAX / 2); //zFar

      m_camera.eye = { 50.0f, 50.0f, 75.0f };
      m_camera.center = { 50.0f, 50.0f, 0.0f };
      m_camera.up = { 0.0f, 1.0, 0.0f };
   }

   

public:
   Impl(Renderer &r, Window *w):m_renderer(r), m_window(w) {}

   void onStartup() {
      m_testModel = buildTestModel();
      m_testShader = ShaderManager::create("assets/shaders.glsl", DiffuseTexture);
      
      TextureRequest request(internString("assets/00.png"));
      m_testTexture = TextureManager::get(request);

      m_testUBO = UBOManager::create(sizeof(Matrix));
      m_renderer.bindUBO(m_testUBO, 0);

      buildCamera();
   }

   void onShutdown() {

   }

   void update() {
      static int i = 0;
      int a = (i++) % 360;
      float rad = a*DEG2RAD;
      float r = 50.0f;

      m_camera.eye.x = 50.0f + r * cos(rad);
      m_camera.eye.z = r * sin(rad);


   }

   void render() {
      Renderer &r = m_renderer;

      auto uModel = internString("uModelMatrix");
      auto uColor = internString("uColorTransform");
      auto uTexture = internString("uTexMatrix");
      auto uTextureSlot = internString("uTexture");

      r.viewport({ 0, 0, (int)r.getWidth(), (int)r.getHeight() });
      r.clear({ 0.0f, 0.0f, 0.0f, 1.0f });

      Matrix modelTransform = Matrix::scale3f({ 100.0f, 100.0f, 50.0f });

      Matrix lookAt = Matrix::lookAt(m_camera.eye, m_camera.center, m_camera.up);
      Matrix viewTransform = m_camera.perspective * lookAt;

      Matrix texTransform = Matrix::identity();

      ColorRGBAf colorTransform = { 0.0f, 1.0f, 1.0f, 1.0f };

      r.setUBOData(m_testUBO, &viewTransform);

      r.setShader(m_testShader);
      r.setMatrix(uModel, modelTransform);
      r.setMatrix(uTexture, texTransform);
      r.setColor(uColor, colorTransform);

      r.bindTexture(m_testTexture, 0);
      r.setTextureSlot(uTextureSlot, 0);

      r.renderModel(m_testModel);

      r.finish();
      r.flush();
   }

   void onStep() {
      update();
      render();
   }
};

Game::Game(Renderer &r, Window *w):pImpl(new Impl(r, w)) {}
Game::~Game() {}
void Game::onStartup() { pImpl->onStartup(); }
void Game::onStep() { pImpl->onStep(); }
void Game::onShutdown() { pImpl->onShutdown(); }