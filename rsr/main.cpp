#include "Window.hpp"

int main()
{
   auto win = Window::create(640, 480, "Test!", 0);

   if (!win || win->beginRender()) {
      return 0;
   }

   while (!win->shouldClose()) {
      win->pollEvents();
      win->swapBuffers();
   }

   Window::destroy(win);
}