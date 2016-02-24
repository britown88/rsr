#pragma once

#include <memory>

#include "Renderer.hpp"
#include "Window.hpp"

class Game {
   class Impl;
   std::unique_ptr<Impl> pImpl;
public:
   Game(Renderer &r, Window *w);
   ~Game();

   void onStartup();
   void onStep();
   void onShutdown();
};
