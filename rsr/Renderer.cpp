#include "GL/glew.h"
#include "Renderer.hpp"

#include "DrawQueue.hpp"

#include <mutex>
#include <vector>


class Renderer::Impl {
   std::shared_ptr<DrawQueue> m_workingQueue, m_drawQueue;
   mutable std::mutex m_mutex;

   Shader *m_activeShader;
   Model *m_activeModel;

   Window *m_wnd;

   template <typename L>
   void draw(L && lambda) {
      m_workingQueue->push(std::move(lambda));
   }

   std::shared_ptr<DrawQueue> getQueue() const {
      m_mutex.lock();
      auto out = m_drawQueue;
      m_mutex.unlock();
      return out;
   }

public:
   Impl(Window *wnd):
      m_wnd(wnd),
      m_workingQueue(new DrawQueue),
      m_drawQueue(new DrawQueue),
      m_activeShader(nullptr),
      m_activeModel(nullptr) {}

   size_t getWidth() const { return m_wnd->getWidth(); }
   size_t getHeight() const { return m_wnd->getHeight(); }

   void finish() {
      //Swap Queues
      m_mutex.lock();
      m_drawQueue = std::move(m_workingQueue);
      m_workingQueue.reset(new DrawQueue());
      m_mutex.unlock();
   }

   void flush() const {
      getQueue()->draw();
      m_wnd->swapBuffers();
   }

   void beginRender() const {
      m_wnd->beginRender();
      glewInit();

      glLineWidth(3.0f);
      glPointSize(1.0f);

      
   }

   void enableDepth(bool enabled) {
      draw([=]() {

         if (enabled) {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glAlphaFunc(GL_GREATER, 0.5);
            glEnable(GL_ALPHA_TEST);
         }
         else {
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_ALPHA_TEST);
         }
      });
   }

   void enableAlphaBlending(bool enabled) {
      draw([=]() {

         if (enabled) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         }
         else {
            glDisable(GL_BLEND);
         }
      });
   }
   void enableWireframe(bool enabled) {
      draw([=]() {

         if (enabled) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
         }
         else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
         }
      });
   }

   //render functions
   void clear(ColorRGBAf const &c) {
      draw([=]() {
         glClearColor(c.r, c.g, c.b, c.a);
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      });
   }
   void viewport(Recti const &r) {
      int winHeight = m_wnd->getHeight();
      Recti bounds = {
         r.top.x,
         winHeight - r.top.y - height(r),
         width(r),
         height(r)
      };

      draw([=]() {
         glViewport(bounds.top.x, bounds.top.y, bounds.bot.x, bounds.bot.y);
      });
   }

   void setShader(Shader *s) {
      draw([=]() {
         if (s != m_activeShader) {
            ShaderManager::setActive(s);
            m_activeShader = s;
         }
      });
   }
   void setFloat2(StringView u, Float2 const &value) {
      draw([=]() {
         if (m_activeShader) {
            auto uni = ShaderManager::getUniform(m_activeShader, u);
            ShaderManager::setFloat2(m_activeShader, uni, value);
         }
      });
   }
   void setMatrix(StringView u, Matrix const &value) {
      draw([=]() {
         if (m_activeShader) {
            auto uni = ShaderManager::getUniform(m_activeShader, u);
            ShaderManager::setMatrix(m_activeShader, uni, value);
         }
      });
   }
   void setColor(StringView u, ColorRGBAf const &value) {
      draw([=]() {
         if (m_activeShader) {
            auto uni = ShaderManager::getUniform(m_activeShader, u);
            ShaderManager::setColor(m_activeShader, uni, value);
         }
      });
   }

   void setTextureSlot(StringView u, TextureSlot const &value) {
      draw([=]() {
         if (m_activeShader) {
            auto uni = ShaderManager::getUniform(m_activeShader, u);
            ShaderManager::setTextureSlot(m_activeShader, uni, value);
         }
      });
   }

   void bindTexture(Texture *t, TextureSlot slot) {
      draw([=]() {
         TextureManager::bind(t, slot);
      });
   }

   void setUBOData(UBO *ubo, size_t offset, size_t size, void *data) {
      draw([=]() {
         UBOManager::setData(ubo, offset, size, data);
      });
   }

   void bindUBO(UBO *ubo, UBOSlot slot) {
      draw([=]() {
         UBOManager::bind(ubo, slot);
      });
   }

   void bindCubeMap(CubeMap *cm, TextureSlot slot) {
      draw([=]() {
         CubeMapManager::bind(cm, slot);
      });
   }

   void renderModel(Model *m, ModelManager::RenderType type) {
      draw([=]() {
         if (m != m_activeModel) {
            ModelManager::bind(m);
            m_activeModel = m;
         }

         ModelManager::draw(m, type);
      });
   }

};

Renderer::Renderer(Window *wnd) :pImpl(new Impl(wnd)){}
Renderer::~Renderer() {}

//utility
void Renderer::finish() { pImpl->finish(); }
void Renderer::flush() const { pImpl->flush(); }
void Renderer::beginRender() const { pImpl->beginRender(); }
size_t Renderer::getWidth() const { return pImpl->getWidth(); }
size_t Renderer::getHeight() const { return pImpl->getHeight(); }

//render functions
void Renderer::clear(ColorRGBAf const &c) { pImpl->clear(c); }
void Renderer::viewport(Recti const &r) { pImpl->viewport(r); }
void Renderer::setShader(Shader *s) { pImpl->setShader(s); }
void Renderer::setFloat2(StringView u, Float2 const &value) { pImpl->setFloat2(u, value); }
void Renderer::setMatrix(StringView u, Matrix const &value) { pImpl->setMatrix(u, value); }
void Renderer::setColor(StringView u, ColorRGBAf const &value) { pImpl->setColor(u, value); }

void Renderer::enableDepth(bool enabled) { pImpl->enableDepth(enabled); }
void Renderer::enableAlphaBlending(bool enabled) { pImpl->enableAlphaBlending(enabled); }
void Renderer::enableWireframe(bool enabled) { pImpl->enableWireframe(enabled); }

void Renderer::setTextureSlot(StringView u, TextureSlot const &value){pImpl->setTextureSlot(u, value);}
void Renderer::bindTexture(Texture *t, TextureSlot slot){pImpl->bindTexture(t, slot);}

void Renderer::_setUBOData(UBO *ubo, size_t offset, size_t size, void *data) { pImpl->setUBOData(ubo, offset, size, data); }
void Renderer::bindUBO(UBO *ubo, UBOSlot slot) { pImpl->bindUBO(ubo, slot); }
void Renderer::bindCubeMap(CubeMap *cm, TextureSlot slot) { pImpl->bindCubeMap(cm, slot); }

void Renderer::renderModel(Model *m, ModelManager::RenderType type) { pImpl->renderModel(m, type); }