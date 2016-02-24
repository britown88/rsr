#include "Window.hpp"
#include "Renderer.hpp"
#include "Game.hpp"

int main()
{
   Window *win = Window::create(640, 480, "Test!", 0);

   if (!win) {
      return 0;
   }

   Renderer r(win);
   Game g(r, win);

   r.beginRender();
   g.onStartup();   

   while (!win->shouldClose()) {      
      g.onStep();
      win->pollEvents();
   }

   g.onShutdown();
   Window::destroy(win);
}