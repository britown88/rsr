#include "Input.hpp"

#include <vector>
#include <functional>

using namespace Input;

class Mouse::Impl {
   std::vector<MouseEvent> m_eventQueue;   
   PositionGet m_getPos;
   bool m_heldMap[MouseBtn_COUNT];
   int m_queuePos;
public:
   Impl(PositionGet const &posGet):m_getPos(posGet) {}
   ~Impl(){}

   void pushEvent(MouseEvent const &e) { m_eventQueue.push_back(e); }
   ::Int2 position() { return m_getPos(); }

   MouseEvent *popEvent() {
      if (m_queuePos == m_eventQueue.size()) {
         return nullptr;
      }

      auto out = &m_eventQueue[m_queuePos++];

      if (out->action == Key_Pressed) {
         m_heldMap[out->button] = true;
      }
      else if (out->action == Key_Released) {
         m_heldMap[out->button] = false;
      }

      return out;
   }

   bool isDown(MouseButtons button) {
      return m_heldMap[button];
   }

   void flushQueue() {
      while (popEvent());
      m_queuePos = 0;
      m_eventQueue.clear();
   }
};

Mouse::Mouse(PositionGet const &posGet) :pImpl(new Impl(posGet)) {}
Mouse::~Mouse() {}
Mouse *Mouse::create(PositionGet const &posGet) { return new Mouse(posGet); }
void Mouse::destroy(Mouse *self) { delete self; }

void Mouse::pushEvent(MouseEvent const &e) { pImpl->pushEvent(e); }
Int2 Mouse::position() { return pImpl->position(); }

MouseEvent *Mouse::popEvent() { return pImpl->popEvent(); }
bool Mouse::isDown(MouseButtons button) { return pImpl->isDown(button); }
void Mouse::flushQueue() { pImpl->flushQueue(); }


class Keyboard::Impl {
   std::vector<KeyboardEvent> m_eventQueue;
   bool m_heldMap[Key_COUNT];
   int m_queuePos;

   KeyMods getMods() {
      int out = 0;

      if (isDown(Key_LeftShift) || isDown(Key_RightShift)) {
         out |= Key_ModShift;
      }

      if (isDown(Key_LeftAlt) || isDown(Key_RightAlt)) {
         out |= Key_ModAlt;
      }

      if (isDown(Key_LeftControl) || isDown(Key_RightControl)) {
         out |= Key_ModCtrl;
      }

      return (KeyMods)out;
   }

public:
   Impl() {}
   ~Impl() {}

   void pushEvent(KeyboardEvent const &e) { m_eventQueue.push_back(e); }

   KeyboardEvent *popEvent() {
      if (m_queuePos == m_eventQueue.size()) {
         return nullptr;
      }

      auto out = &m_eventQueue[m_queuePos++];

      out->mods = getMods();

      if (out->action == Key_Pressed) {
         m_heldMap[out->key] = true;
      }
      else if (out->action == Key_Released) {
         m_heldMap[out->key] = false;
      }

      return out;
   }
   bool isDown(Keys key) { return m_heldMap[key]; }
   void flushQueue() {
      while (popEvent());
      m_queuePos = 0;
      m_eventQueue.clear();
   }

};

Keyboard::Keyboard():pImpl(new Impl()) {}
Keyboard::~Keyboard() {}

Keyboard *Keyboard::create() { return new Keyboard(); }
void Keyboard::destroy(Keyboard *self) { delete self; }

void Keyboard::pushEvent(KeyboardEvent const &e) { pImpl->pushEvent(e); }

KeyboardEvent *Keyboard::popEvent() { return pImpl->popEvent(); }
bool Keyboard::isDown(Keys key) { return pImpl->isDown(key); }
void Keyboard::flushQueue() { pImpl->flushQueue(); }