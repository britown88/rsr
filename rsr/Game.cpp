#include "Game.hpp"
#include "Camera.hpp"
#include "CubeMap.hpp"
#include "Track.hpp"

#include <algorithm>

namespace Shaders {
   static Shader *Skybox = nullptr;
   static Shader *Wireframe = nullptr;
   static Shader *RWireframe = nullptr;
   static Shader *Lines = nullptr;
   static Shader *RLines = nullptr;
   static Shader *Bunny = nullptr;
   static Shader *Track = nullptr;

   static void build() {
      Skybox = ShaderManager::create("assets/skybox.glsl");
      Wireframe = ShaderManager::create("assets/wireframe.glsl");
      RWireframe = ShaderManager::create("assets/wireframe.glsl", Rotation);
      Lines = ShaderManager::create("assets/shaders.glsl", ColorAttribute);
      RLines = ShaderManager::create("assets/shaders.glsl", ColorAttribute | Rotation);
      Bunny = ShaderManager::create("assets/shaders.glsl", DiffuseLighting | Rotation);
      Track = ShaderManager::create("assets/shaders.glsl", DiffuseLighting);
   }
}

struct BunnyModel {
   ModelVertices vertices;
   Model *renderModel;
   QuickHullTestModels hullModels;
};

struct Bunny {
   const float WheelTurnRate = 1.0f;
   const float ThrottleRate = 0.01f;
   const float MaxTurnAngle = 1.0f;
   const float MaxThrottle = 0.5f;

   Float3 pos, scale;
   ColorRGBAf color;

   Matrix modelMatrix, rotation, debugLinesMatrix;

   std::vector<FVF_Pos3_Col4> debugLines;
   Model *debugLinesModel;

   enum Lines {
      Forward = 0,
      Up = 2,
      Wheelbase = 4,
      FrontAxis = 6,
      BackAxis = 8,
      LineCount = 10
   };

   void createDebugLines() {
      debugLines.resize(LineCount, { {0.0f, 0.0f, 0.0f}, CommonColors::White});

      debugLines[Forward].col4 = debugLines[Forward+1].col4 = CommonColors::Red;
      debugLines[Up].col4 = debugLines[Up + 1].col4 = CommonColors::Green;
      //debugLines[Wheelbase].col4 = debugLines[Wheelbase + 1].col4 = CommonColors::Blue;
      debugLines[FrontAxis].col4 = debugLines[FrontAxis + 1].col4 = CommonColors::Blue;
      debugLines[BackAxis].col4 = debugLines[BackAxis + 1].col4 = CommonColors::Blue;

      debugLinesModel = ModelManager::create(debugLines, ModelManager::Stream);
   }

   struct Control {
      bool left = false;
      bool right = false;
      bool forward = false;
      bool back = false;
   };

   Control control;

   Float3 velocity, forward, up;
   float wheelBase;
   float steeringAngle;
   float throttle;

   void updateDebugLines() {
      debugLines[Forward + 1].pos3 = vec::mul(forward, 0.15f);
      debugLines[Up + 1].pos3 = vec::mul(up, 0.15f);

      auto frontCenter = vec::mul(forward, wheelBase * 0.5f);
      auto backCenter = vec::mul(forward, -wheelBase * 0.5f);
      auto axis = vec::normal(vec::cross(forward, up));
      auto frontAxis = Quaternion::fromAxisAngle(up, steeringAngle * 10.0f).rotate(axis);

      debugLines[FrontAxis].pos3 = vec::add(frontCenter, vec::mul(frontAxis, 0.15f));
      debugLines[FrontAxis + 1].pos3 = vec::sub(frontCenter, vec::mul(frontAxis, 0.15f));

      debugLines[BackAxis].pos3 = vec::add(backCenter, vec::mul(axis, 0.15f));
      debugLines[BackAxis + 1].pos3 = vec::sub(backCenter, vec::mul(axis, 0.15f));


      ModelManager::updateData(debugLinesModel, debugLines);
   }

   void updateThrottle() {
      if (control.forward) {
         throttle = std::min(MaxThrottle, throttle + ThrottleRate);
      }
      else if (control.back) {
         throttle = std::max(-MaxThrottle, throttle - ThrottleRate);
      }
      else {
         if (throttle > 0.0001f) {
            throttle = std::max(0.0f, throttle - ThrottleRate);
         }
         if (throttle < -0.0001f) {
            throttle = std::min(0.0f, throttle + ThrottleRate);
         }
      }

   }

   void updateTurnAngle() {
      if (control.left) {
         steeringAngle = std::min(MaxTurnAngle, steeringAngle + WheelTurnRate);
      }
      else if (control.right) {
         steeringAngle = std::max(-MaxTurnAngle, steeringAngle - WheelTurnRate);
      }
      else {
         if (steeringAngle > 0.0001f) {
            steeringAngle = std::max(0.0f, steeringAngle - WheelTurnRate);
         }
         else if (steeringAngle < -0.0001f) {
            steeringAngle = std::min(0.0f, steeringAngle + WheelTurnRate);
         }
      }
   }

   void updatePosition() {

      if (fabs(throttle) > 0.001f) {
         auto frontCenter = vec::mul(forward, wheelBase * 0.5f);
         auto backCenter = vec::mul(forward, -wheelBase * 0.5f);

         auto frontDir = Quaternion::fromAxisAngle(up, steeringAngle).rotate(forward);

         frontCenter = vec::add(frontCenter, vec::mul(frontDir, throttle));
         backCenter = vec::add(backCenter, vec::mul(forward, throttle));

         forward = vec::normal(vec::sub(frontCenter, backCenter));
         pos = vec::add(vec::add(pos, backCenter), vec::mul(forward, wheelBase * 0.5f));
      }
      
   }

   void updateMatrix() {
      debugLinesMatrix = Matrix::translate3f(pos)  * Matrix::scale3f(scale);//unrotated
      rotation = Matrix::fromBasis(forward, up, vec::cross(forward, up));
      modelMatrix = debugLinesMatrix;
   }

   void update() {
      updateThrottle();
      updateTurnAngle();

      //up = Quaternion::fromAxisAngle(forward, 1.0f).rotate(up);

      updatePosition();
      updateDebugLines();
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

   float m_axisScale = 10.0f;

   Model *m_skybox, *m_testTrack, *m_axisLines;
   UBO *m_testUBO;
   CubeMap *m_cubemap;
   TestUBO m_u;

   CameraControl m_camera;

   BunnyModel m_bunnyModel;
   Bunny m_bunny;

   int qhIterCount = 0;

   void buildBunnyModel() {

      auto vertexSet = ModelVertices::fromOBJ("assets/bunny.obj");
      if (!vertexSet.empty()) {
         auto &vs = vertexSet[0];
         auto q = Quaternion::fromAxisAngle({ 0.0f, 1.0f, 0.0f }, 180.0f);
         int i = 0;
         for (auto &p : vs.positions) {
            p.y -= 0.075f;
            p.z -= 0.01f;
            p = q.rotate(p);

            //if (p.y > 0.03f && p.x > 0.05) {
            //   p = vec::sub(p, { 0.1f, 0.0f, 0.0f });
            //   p = Quaternion::fromAxisAngle({ 0.0f, 1.0f, 0.0f }, -45.0f).rotate(p);
            //   p = vec::add(p, { 0.1f, 0.0f, 0.0f });
            //   p.z += 0.02f;
            //}
         }

         m_bunnyModel.hullModels = quickHullTest(vs.positions, qhIterCount);


         m_bunnyModel.vertices = vs.calculateNormals();
         m_bunnyModel.renderModel = vs.expandIndices().createModel(ModelOpts::IncludeNormals);

         
      }
   }

   void buildBunny() {      
      m_bunny.pos = { -50.0f, 1.0f, -50.0f };
      m_bunny.scale = vec::mul({ 1.0f, 1.0f, 1.0f }, 20.0f);
      m_bunny.color = CommonColors::Green;

      m_bunny.forward = { 0.0f, 0.0f, -1.0f };
      m_bunny.up = { 0.0f, 1.0f, 0.0f };
      m_bunny.wheelBase = 0.25f;

      m_bunny.createDebugLines();
      m_bunny.update();
      
   }
   
   void buildTestLines() {
      std::vector<FVF_Pos3_Col4> vertices = {
         { { -0.5f, 0.0f, 0.0f }, CommonColors::DkRed },
         { {  1.0f, 0.0f, 0.0f }, CommonColors::Red },

         { { 0.0f, -0.5f, 0.0f }, CommonColors::DkGreen },
         { { 0.0f,  1.0f, 0.0f }, CommonColors::Green },

         { { 0.0f, 0.0f, -0.5f }, CommonColors::DkBlue },
         { { 0.0f, 0.0f,  1.0f }, CommonColors::Blue }
      };

      m_axisLines = ModelManager::create(vertices);
   }

   void buildSkybox() {
      auto vertexSet = ModelVertices::fromOBJ("assets/myshittyskybox.obj");
      if (!vertexSet.empty()) {
         m_skybox = vertexSet[0].expandIndices().createModel();
      }

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
         { { -50.0f, 0.0f, -50.0f },   0.0f, 10.0f },
         { {  50.0f, 0.0f,   -50.0f }, 20.0f, 10.0f },
         { {  50.0f, 0.0f,  50.0f },   45.0f, 10.0f },
         { { -50.0f, 0.0f,    50.0f }, 20.0f, 10.0f },
         { { -50.0f, 0.0f, -50.0f },   0.0f, 10.0f }
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
      m_camera.distance = 20.0f;

      //m_axisScale = m_camera.distance / 10.0f;

      m_camera.update();
   }
   

public:
   Impl(Renderer &r, Window *w):m_renderer(r), m_window(w) {}

   void onStartup() {
      Shaders::build();
      
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
         case Keys::Key_KeypadAdd:
            if (ke->action == KeyActions::Key_Pressed) {
               
               qhIterCount++;
               buildBunnyModel();
            }
            
            break;
         case Keys::Key_KeypadSubtract:
            if (ke->action == KeyActions::Key_Pressed && qhIterCount > 0) {
               qhIterCount++;
               buildBunnyModel();
            }
            break;
         }
         
      }

      m_bunny.control.left = k->isDown(Keys::Key_A);
      m_bunny.control.right = k->isDown(Keys::Key_D);
      m_bunny.control.forward = k->isDown(Keys::Key_W);
      m_bunny.control.back = k->isDown(Keys::Key_S);

      k->flushQueue();
   }

   void updateMouse(Mouse *m) {
      while (auto me = m->popEvent()) {
         switch (me->action) {

         case MouseActions::Mouse_Scrolled:
            m_camera.distance += -me->pos.y * 0.05f;
            m_camera.distance = std::min(1000.0f, std::max(1.0f, m_camera.distance));
            //m_axisScale = m_camera.distance / 10.0f;
            break;
         case MouseActions::Mouse_Moved:
            if (m->isDown(MouseButtons::MouseBtn_Right)) {
               m_camera.coords.dip = std::min(89.99f, std::max(-89.99f, m_camera.coords.dip + me->pos.y));
               m_camera.coords.azm += me->pos.x;
            }
            break;
         }
      }
      m->flushQueue();
   }

   void update() {
      static int j = 0;

      m_bunny.update();
      m_camera.followPoint = m_bunny.pos;
      m_camera.update();      

      updateKeyboard(m_window->getKeyboard());
      updateMouse(m_window->getMouse());
   }

   void render() {
      Renderer &r = m_renderer;

      auto uModel = internString("uModelMatrix");
      auto uModelRotation = internString("uModelRotation");
      auto uColor = internString("uColorTransform");
      auto uTexture = internString("uTexMatrix");
      auto uTextureSlot = internString("uTexture");
      auto uSkyboxSlot = internString("uSkybox");

      

      Matrix texTransform = Matrix::identity();


      r.viewport({ 0, 0, (int)r.getWidth(), (int)r.getHeight() });
      r.clear(CommonColors::Black);

      TestUBO cameraUbo;
      cameraUbo.c = m_camera.cam;
      cameraUbo.view = cameraUbo.c.perspective * Matrix::lookAt(Float3(), vec::sub(cameraUbo.c.center, cameraUbo.c.eye), cameraUbo.c.up);
      r.setUBOData(m_testUBO, cameraUbo);

      r.enableDepth(false);

      r.setShader(Shaders::Skybox);
      r.bindCubeMap(m_cubemap, 0);
      r.setTextureSlot(uSkyboxSlot, 0);
      //r.renderModel(m_skybox);

      r.enableDepth(true);

      m_u.c = m_camera.cam;
      Matrix lookAt = Matrix::lookAt(m_u.c.eye, m_u.c.center, m_u.c.up);
      m_u.view = m_u.c.perspective * lookAt;
      m_u.light = { 0.0f, -1.0f, 0.0f };
      m_u.ambient = 0.1f;

      r.setUBOData(m_testUBO, m_u);
      r.enableAlphaBlending(true);

      r.setShader(Shaders::Lines);
      r.setMatrix(uModel, Matrix::scale3f(vec::mul({ 1.0f, 1.0f, 1.0f }, m_axisScale)));
      r.setColor(uColor, CommonColors::White);
      r.renderModel(m_axisLines, ModelManager::Lines);

      r.setShader(Shaders::Bunny);
      r.setMatrix(uModel, m_bunny.modelMatrix);
      r.setMatrix(uModelRotation, m_bunny.rotation);
      r.setColor(uColor, m_bunny.color);
      //r.renderModel(m_bunnyModel.renderModel);

      r.setShader(Shaders::Lines);

      r.setMatrix(uModel, m_bunny.debugLinesMatrix);
      r.setColor(uColor, CommonColors::White);
      //r.renderModel(m_bunny.debugLinesModel, ModelManager::Lines);


      r.setShader(Shaders::Track);
      r.setMatrix(uModel, Matrix::identity());
      r.setColor(uColor, CommonColors::DkGray);
      //r.renderModel(m_testTrack);

      //r.enableDepth(false);

      r.setShader(Shaders::RLines);
      r.setMatrix(uModel, m_bunny.modelMatrix);
      r.setMatrix(uModelRotation, m_bunny.rotation);
      r.setColor(uColor, CommonColors::White);
      
      for (auto m : m_bunnyModel.hullModels.lineModels) {
         r.renderModel(m, ModelManager::Lines);
      }

      for (auto m : m_bunnyModel.hullModels.pointModels) {
         r.renderModel(m, ModelManager::Points);
      }
      //
      //r.enableDepth(true);

      //wireframes
      //r.enableWireframe(true);
      //r.setShader(Shaders::RWireframe);
      //r.setColor(uColor, CommonColors::Cyan);

      //r.setMatrix(uModel, m_bunny.modelMatrix);
      //r.setMatrix(uModelRotation, m_bunny.rotation);
      //r.renderModel(m_bunnyModel.renderModel);

      //r.setShader(Shaders::Wireframe);
      //r.setColor(uColor, CommonColors::Cyan);
      //r.setMatrix(uModel, Matrix::identity());
      //r.renderModel(m_testTrack);

      //r.enableWireframe(false);

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