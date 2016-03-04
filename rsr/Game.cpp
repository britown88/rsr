#include "Game.hpp"
#include "Camera.hpp"
#include "CubeMap.hpp"
#include "Track.hpp"

struct TestUBO {
   Matrix view;
   Float3 light;
   float ambient;
   Camera c;
};

class Game::Impl {
   Renderer &m_renderer;
   Window *m_window;

   Model *m_testModel, *m_msb, *m_testTrack;
   Shader *m_testShader, *m_mshader;
   Texture *m_testTexture, *m_sbtex;
   UBO *m_testUBO;
   CubeMap *m_cubemap;
   TestUBO m_u;

   Camera m_camera;

   void buildSkybox() {
      m_msb = ModelManager::importFromOBJ("assets/myshittyskybox.obj");
      m_mshader = ShaderManager::create("assets/skybox.glsl", 0);
      m_cubemap = CubeMapManager::create({
         "assets/skybox3/right.png", 
         "assets/skybox3/left.png" , 
         "assets/skybox3/top.png" , 
         "assets/skybox3/bottom.png" , 
         "assets/skybox3/back.png" , 
         "assets/skybox3/front.png" });

   }

   Model *buildTestTrack() {
      std::vector<TrackPoint> pointList = {
         { { -50.0f, -100.0f, -50.0f },   0.0f, 10.0f },
         { {  50.0f, -100.0f,   -50.0f }, 0.25f, 10.0f },
         { {  50.0f, -100.0f,  50.0f },   0.9f, 10.0f },
         { { -50.0f, -100.0f,    50.0f }, 0.25f, 10.0f },
         { { -50.0f, -100.0f, -50.0f },   0.0f, 10.0f }
      };

      return createTrackSegment(pointList, true);
   }

   Model *buildTestModel() {
      std::vector<FVF_Pos3_Tex2_Col4> vertices = {
         { { -0.5f, -0.5f, 0.0f },{ 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f, 1.0f } },
         { { 0.5f, -0.5f, 0.0f },{ 1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } },
         { { 0.5f, 0.5f, 0.0f },{ 1.0f, 1.0f },{ 0.0f, 0.0f, 1.0f, 1.0f } },
         { { -0.5f, 0.5f, 0.0f },{ 0.0f, 1.0f },{ 1.0f, 1.0f, 1.0f, 1.0f } }
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
         60.0f, //fov                                       
         aspectRatio, //aspect ratio
         0.01f, // zNear
         FLT_MAX / 2); //zFar

      m_camera.eye = { 0.0f, 0.0f, 0.0f };
      m_camera.center = { 0.0f, 0.0f, 0.0f };
      m_camera.up = { 0.0f, 1.0, 0.0f };
   }

   static const int testBakers = 1;

   Matrix testBakerModels[testBakers];
   ColorRGBAf testBakerColors[testBakers];
   

public:
   Impl(Renderer &r, Window *w):m_renderer(r), m_window(w) {}

   void onStartup() {
      m_testModel = ModelManager::importFromOBJ("assets/dragon.obj");

      //m_testModel = buildTestModel();
      m_testShader = ShaderManager::create("assets/shaders.glsl", DiffuseLighting);      
      TextureRequest request(internString("assets/granite.png"), Repeat);
      m_testTexture = TextureManager::get(request);

      m_testUBO = UBOManager::create(sizeof(TestUBO));
      m_renderer.bindUBO(m_testUBO, 0);

      buildCamera();

      float size = 9.0f;

      for (int i = 0; i < testBakers; ++i) {
         testBakerModels[i] =
            //Matrix::translate3f({ (float)((rand() % 300) - 150), (float)((rand() % 300) - 150), (float)((rand() % 300) - 150) }) *
            //Matrix::translate3f({0.0f, -80.0f, 0.0f}) * 
            Matrix::scale3f({ size, size, size });

         testBakerColors[i] = { (rand()%100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f, 1.0f };
      }

      buildSkybox();

      m_testTrack = buildTestTrack();
   }

   void onShutdown() {

   }

   void update() {
      static int j = 0;

      if (j++ % 5 == 0) {
         static int i = 0;
         int a = ((i++) % 360);
         float rad = a*DEG2RAD;
         float r = 250.0f;

         m_camera.eye.x = r * cos(rad);
         m_camera.eye.z = r * sin(rad);
         m_camera.eye.y = m_camera.eye.z / 2.5f;
      }


      


   }

   void render() {
      Renderer &r = m_renderer;

      auto uModel = internString("uModelMatrix");
      auto uColor = internString("uColorTransform");
      auto uTexture = internString("uTexMatrix");
      auto uTextureSlot = internString("uTexture");
      auto uSkyboxSlot = internString("uSkybox");

      

      Matrix texTransform = Matrix::identity();


      r.viewport({ 0, 0, (int)r.getWidth(), (int)r.getHeight() });
      r.clear({ 0.0f, 0.0f, 0.0f, 1.0f });

      TestUBO cameraUbo;
      cameraUbo.view = m_camera.perspective * Matrix::lookAt(Float3(), vec::sub(m_camera.center, m_camera.eye), m_camera.up);
      m_camera.dir = vec::normal(vec::sub(m_camera.center, m_camera.eye));
      cameraUbo.c = m_camera;
      r.setUBOData(m_testUBO, cameraUbo);

      r.enableDepth(false);

      r.setShader(m_mshader);
      r.bindCubeMap(m_cubemap, 0);
      r.setTextureSlot(uSkyboxSlot, 0);
      r.renderModel(m_msb);


      r.enableDepth(true);

      Matrix lookAt = Matrix::lookAt(m_camera.eye, m_camera.center, m_camera.up);
      m_u.view = m_camera.perspective * lookAt;
      m_u.light = { 0.0f, -1.0f, 0.0f };
      m_u.ambient = 0.1f;

      m_camera.dir = vec::normal(vec::sub(m_camera.center, m_camera.eye));
      m_u.c = m_camera;

      r.setUBOData(m_testUBO, m_u);

      r.setShader(m_testShader);
      for (int i = 0; i < testBakers; ++i) {
         r.setMatrix(uModel, testBakerModels[i]);
         r.setMatrix(uTexture, texTransform);
         r.setColor(uColor, testBakerColors[i]);

         r.bindTexture(m_testTexture, 0);
         r.setTextureSlot(uTextureSlot, 0);

         r.renderModel(m_testModel);
      }

      r.setMatrix(uModel, Matrix::identity());
      r.setColor(uColor, CommonColors::White);

      r.renderModel(m_testTrack);

      

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