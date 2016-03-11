#include "Game.hpp"
#include "Camera.hpp"
#include "CubeMap.hpp"
#include "Track.hpp"

#include <algorithm>

struct BunnyModel {
   Shader *shader;
   ModelVertices vertices;
   Model *renderModel;
};

struct Bunny {
   const float wheelTurnRate = 1.0f;

   Float3 pos, scale;
   ColorRGBAf color;

   Matrix modelMatrix;

   struct Control {
      bool left = false;
      bool right = false;
      bool forward = false;
      bool back = false;
   };

   Control control;

   Float3 velocity, forward, up;
   float turnAngle;

   void updateTurnAngle() {
      if (control.left) {
         turnAngle = std::min(45.0f, turnAngle + wheelTurnRate);
      }
      else if (control.right) {
         turnAngle = std::min(45.0f, turnAngle - wheelTurnRate);
      }
      else {
         if (turnAngle > 0.0001f) {
            turnAngle = std::max(0.0f, turnAngle - wheelTurnRate);
         }
         else if (turnAngle < -0.0001f) {
            turnAngle = std::min(0.0f, turnAngle + wheelTurnRate);
         }
      }
   }

   void updateMatrix() {
      modelMatrix = Matrix::translate3f(pos)  * Matrix::scale3f(scale) * Matrix::fromBasis(forward, up, vec::cross(forward, up));
   }

   void update() {
      updateTurnAngle();
      updateMatrix();
   }
};

struct TestUBO {
   Matrix view;
   Float3 light;
   float ambient;
   Camera c;
};

struct CameraControl {
   Camera cam;
   Spherical coords;
   Float3 followPoint;
   float distance;

   void update() {
      cam.center = followPoint;
      cam.eye = vec::add(vec::mul(Spherical::toCartesian(coords), distance), cam.center);
      cam.dir = vec::normal(vec::sub(cam.center, cam.eye));
   }
};

class Game::Impl {
   Renderer &m_renderer;
   Window *m_window;

   float m_axisScale = 1.0f;

   Model *m_skybox, *m_testTrack, *m_axisLines;
   Shader *m_skyboxShader, *m_wireframeShader, *m_lineShader;
   UBO *m_testUBO;
   CubeMap *m_cubemap;
   TestUBO m_u;

   CameraControl m_camera;

   BunnyModel m_bunnyModel;
   Bunny m_bunny;

   void buildBunnyModel() {
      m_bunnyModel.shader = ShaderManager::create("assets/shaders.glsl", DiffuseLighting);

      auto vertexSet = ModelVertices::fromOBJ("assets/bunny.obj");
      if (!vertexSet.empty()) {
         auto &vs = vertexSet[0];
         auto q = Quaternion::fromAxisAngle({ 0.0f, 1.0f, 0.0f }, 180.0f);
         for (auto &p : vs.positions) {
            p.y -= 0.075f;
            p.z -= 0.01f;
            p = q.rotate(p);
         }

         m_bunnyModel.vertices = vs.calculateNormals();
         m_bunnyModel.renderModel = vs.expandIndices().createModel(ModelOpts::IncludeNormals);
      }
   }

   void buildBunny() {      
      m_bunny.pos = { 0.0f, 0.0f, 0.0f };
      m_bunny.scale = vec::mul({ 1.0f, 1.0f, 1.0f }, 100.0f);
      m_bunny.color = CommonColors::Yellow;

      m_bunny.forward = { 0.0f, 0.0f, -1.0f };
      m_bunny.up = { 0.0f, 1.0f, 0.0f };

      m_bunny.update();
   }


   void buildTestLines() {
      std::vector<FVF_Pos3_Col4> vertices = {
         { { -0.5f, 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f, 1.0f } },
         { {  1.0f, 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f, 1.0f } },

         { { 0.0f, -0.5f, 0.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } },
         { { 0.0f,  1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } },

         { { 0.0f, 0.0f, -0.5f },{ 0.0f, 0.0f, 1.0f, 1.0f } },
         { { 0.0f, 0.0f,  1.0f },{ 0.0f, 0.0f, 1.0f, 1.0f } }
      };

      m_axisLines = ModelManager::create(vertices);
   }

   void buildSkybox() {
      auto vertexSet = ModelVertices::fromOBJ("assets/myshittyskybox.obj");
      if (!vertexSet.empty()) {
         m_skybox = vertexSet[0].expandIndices().createModel();
      }

      m_skyboxShader = ShaderManager::create("assets/skybox.glsl");
      m_cubemap = CubeMapManager::create({
         "assets/skybox3/right.png", 
         "assets/skybox3/left.png" , 
         "assets/skybox3/top.png" , 
         "assets/skybox3/bottom.png" , 
         "assets/skybox3/back.png" , 
         "assets/skybox3/front.png" });

   }

   void buildTestTrack() {
      std::vector<TrackPoint> pointList = {
         { { -50.0f, -100.0f, -50.0f },   0.0f, 10.0f },
         { {  50.0f, -100.0f,   -50.0f }, 20.0f, 10.0f },
         { {  50.0f, -100.0f,  50.0f },   45.0f, 10.0f },
         { { -50.0f, -100.0f,    50.0f }, 20.0f, 10.0f },
         { { -50.0f, -100.0f, -50.0f },   0.0f, 10.0f }
      };

      m_testTrack = createTrackSegment(pointList, true);
   }
   
   void buildCamera() {
      float aspectRatio = (float)m_window->getWidth() / (float)m_window->getHeight();

      m_camera.cam.perspective = Matrix::perspective(
         60.0f, //fov                                       
         aspectRatio, //aspect ratio
         0.01f, // zNear
         FLT_MAX / 2); //zFar

      m_camera.cam.eye = { 0.0f };
      m_camera.cam.center = { 0.0f };
      m_camera.cam.up = { 0.0f, 1.0, 0.0f };

      m_camera.coords = { 1.0f, 0.0f, 90.0f };
      m_camera.followPoint = { 0.0f };
      m_camera.distance = 100.0f;

      m_axisScale = m_camera.distance / 10.0f;

      m_camera.update();
   }
   

public:
   Impl(Renderer &r, Window *w):m_renderer(r), m_window(w) {}

   void onStartup() {
      
   
      m_lineShader = ShaderManager::create("assets/shaders.glsl", ColorAttribute);

      m_wireframeShader = ShaderManager::create("assets/wireframe.glsl");

      m_testUBO = UBOManager::create(sizeof(TestUBO));
      m_renderer.bindUBO(m_testUBO, 0);

      buildCamera();
      buildSkybox();
      buildTestTrack();
      buildTestLines();

      buildBunnyModel();
      buildBunny();
   }

   void onShutdown() {

   }

   void updateKeyboard(Keyboard *k) {
      while (auto ke = k->popEvent()) {
         switch (ke->key) {
         case Keys::Key_Escape:
            m_window->close();
            break;
         }
      }

      k->flushQueue();
   }

   void updateMouse(Mouse *m) {
      while (auto me = m->popEvent()) {
         switch (me->action) {

         case MouseActions::Mouse_Scrolled:
            m_camera.distance += -me->pos.y * 0.05f;
            m_camera.distance = std::min(1000.0f, std::max(1.0f, m_camera.distance));
            m_axisScale = m_camera.distance / 10.0f;
            m_camera.update();
            break;
         case MouseActions::Mouse_Moved:
            if (m->isDown(MouseButtons::MouseBtn_Right)) {
               m_camera.coords.dip = std::min(89.99f, std::max(-89.99f, m_camera.coords.dip + me->pos.y));
               m_camera.coords.azm += me->pos.x;
               m_camera.update();

            }
            break;
         }
      }
      m->flushQueue();
   }

   void update() {
      static int j = 0;

      //if (j++ % 1 == 0) {
      //   static int i = 0;
      //   int a = ((i++) % 360);
      //   float rad = a*DEG2RAD;
      //   float r = 250.0f;

      //   m_camera.eye.x = r * cos(rad);
      //   m_camera.eye.z = r * sin(rad);
      //   m_camera.eye.y = m_camera.eye.z / 2.5f;
      //}


      m_bunny.update();

      updateKeyboard(m_window->getKeyboard());
      updateMouse(m_window->getMouse());
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
      cameraUbo.c = m_camera.cam;
      cameraUbo.view = cameraUbo.c.perspective * Matrix::lookAt(Float3(), vec::sub(cameraUbo.c.center, cameraUbo.c.eye), cameraUbo.c.up);
      r.setUBOData(m_testUBO, cameraUbo);

      r.enableDepth(false);

      r.setShader(m_skyboxShader);
      r.bindCubeMap(m_cubemap, 0);
      r.setTextureSlot(uSkyboxSlot, 0);
      r.renderModel(m_skybox);

      r.enableDepth(true);

      m_u.c = m_camera.cam;
      Matrix lookAt = Matrix::lookAt(m_u.c.eye, m_u.c.center, m_u.c.up);
      m_u.view = m_u.c.perspective * lookAt;
      m_u.light = { 0.0f, -1.0f, 0.0f };
      m_u.ambient = 0.1f;

      r.setUBOData(m_testUBO, m_u);
      r.enableAlphaBlending(true);

      r.setShader(m_lineShader);
      r.setMatrix(uModel, Matrix::scale3f(vec::mul({ 1.0f, 1.0f, 1.0f }, m_axisScale)));
      r.setColor(uColor, CommonColors::White);
      r.renderModel(m_axisLines, ModelManager::Lines);



      r.setShader(m_bunnyModel.shader);
      r.setMatrix(uModel, m_bunny.modelMatrix);
      r.setColor(uColor, m_bunny.color);
      r.renderModel(m_bunnyModel.renderModel);


      r.setMatrix(uModel, Matrix::identity());
      r.setColor(uColor, CommonColors::White);
      r.renderModel(m_testTrack);

      r.enableWireframe(true);
      r.setShader(m_wireframeShader);
      r.setColor(uColor, CommonColors::Cyan);

      //r.setMatrix(uModel, m_bunny.modelMatrix);
      //r.renderModel(m_bunny.model);

      r.setMatrix(uModel, Matrix::identity());
      //r.renderModel(m_testTrack);

      r.enableWireframe(false);

      r.enableAlphaBlending(false);

      

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