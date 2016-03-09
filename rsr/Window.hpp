#pragma once

#include <memory>

#include "Input.hpp"

class Window {
public:
   class Impl;
private:
   std::unique_ptr<Impl> pImpl;

   Window();
   ~Window();
public:
   enum : int {
      FULLSCREEN = 1 << 0
   };

   static Window *create(size_t width, size_t height, const char *title, int flags);
   static void destroy(Window *wnd);

   void close();

   bool shouldClose();
   void pollEvents();

   size_t getWidth();
   size_t getHeight();

   //call in the thread where rendering will take place
   int beginRender();

   void swapBuffers();

   Input::Mouse *getMouse();
   Input::Keyboard *getKeyboard();
};
