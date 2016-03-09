#pragma once

#include "Window.hpp"

#include <windows.h>
#include <GL/GL.h>
#include <GL/GLU.h>

#include <functional>

const char g_szClassName[] = "rsrWin";

//LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
//{
//   switch (msg)
//   {
//   case WM_CLOSE:
//      DestroyWindow(hwnd);
//      break;
//   case WM_DESTROY:
//      PostQuitMessage(0);
//      break;
//   default:
//      return DefWindowProc(hwnd, msg, wParam, lParam);
//   }
//   return 0;
//}

class Window::Impl {
   HWND m_hWnd = NULL;
   HDC m_hdc = NULL;
   HGLRC m_hContext = NULL;
   HINSTANCE m_hInstance = NULL;
   size_t m_width = 0, m_height = 0;
   bool m_shouldClose = false;

   Input::Keyboard *m_keyboard;
   Input::Mouse *m_mouse;

   Int2 getMousePosition() {
      return Int2();
   }

   static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
   {
      switch (msg)
      {
      case WM_CLOSE:
         DestroyWindow(hwnd);
         break;
      case WM_DESTROY:
         PostQuitMessage(0);
         break;
      default:
         return DefWindowProc(hwnd, msg, wParam, lParam);
      }
      return 0;
   }


public:
   ~Impl() {
      wglMakeCurrent(NULL, NULL);
      wglDeleteContext(m_hContext);
      ReleaseDC(m_hWnd, m_hdc);

      Input::Mouse::destroy(m_mouse);
      Input::Keyboard::destroy(m_keyboard);
   }

   int create(size_t width, size_t height, const char *title, int flags) {
      m_hInstance = GetModuleHandle(NULL);

      WNDCLASSEX wc = { 0 };
      wc.cbSize = sizeof(WNDCLASSEX);
      wc.style = CS_OWNDC;
      wc.lpfnWndProc = WndProc;
      wc.hInstance = m_hInstance;
      wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
      wc.hCursor = LoadCursor(NULL, IDC_ARROW);
      wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
      wc.lpszClassName = g_szClassName;
      wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

      if (!RegisterClassEx(&wc)) {
         MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
         return 1;
      }

      DWORD wflags = WS_OVERLAPPEDWINDOW;

      RECT rect = { 0 };
      rect.right = (LONG)width;
      rect.bottom = (LONG)height;
      AdjustWindowRect(&rect, wflags, false);

      m_hWnd = CreateWindowEx(
         WS_EX_CLIENTEDGE,
         g_szClassName,
         title,
         wflags,
         CW_USEDEFAULT, CW_USEDEFAULT, 
         rect.right - rect.left, 
         rect.bottom - rect.top,
         NULL, NULL, m_hInstance, NULL);

      if (m_hWnd == NULL) {
         MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
         return 1;
      }

      ShowWindow(m_hWnd, TRUE);
      UpdateWindow(m_hWnd);

      m_width = width;
      m_height = height;

      m_keyboard = Input::Keyboard::create();
      m_mouse = Input::Mouse::create([=]() {return getMousePosition();});
      
      return 0;
   }

   bool shouldClose() {
      return m_shouldClose;
   }

   void pollEvents() {
      MSG msg;

      while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
      {
         if (msg.message == WM_QUIT) {
            m_shouldClose = true;
            break;
         }

         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
   }

   size_t getWidth() { return m_width; }
   size_t getHeight() { return m_height; }

   int beginRender() {
      PIXELFORMATDESCRIPTOR pfd = { 0 };
      pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
      pfd.nVersion = 1;
      pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
      pfd.iPixelType = PFD_TYPE_RGBA;
      pfd.cColorBits = 32;
      pfd.cDepthBits = 32;
      pfd.iLayerType = PFD_MAIN_PLANE;

      m_hdc = GetDC(m_hWnd);
      int fmt = ChoosePixelFormat(m_hdc, &pfd);

      if (fmt == NULL) {
         MessageBox(NULL, "Failed to choose pixel format!", "Error!", MB_ICONEXCLAMATION | MB_OK);
         return 1;
      }

      if (!SetPixelFormat(m_hdc, fmt, &pfd)) {
         MessageBox(NULL, "Failed to set pixel format!", "Error!", MB_ICONEXCLAMATION | MB_OK);
         return 1;
      }

      m_hContext = wglCreateContext(m_hdc);
      wglMakeCurrent(m_hdc, m_hContext);
      return 0;
   }

   void swapBuffers() {
      SwapBuffers(m_hdc);
   }

   Input::Mouse *getMouse() { return m_mouse; }
   Input::Keyboard *getKeyboard() { return m_keyboard; }
};


Window::Window():pImpl(new Impl()) { }
Window::~Window() {}

bool Window::shouldClose() { return pImpl->shouldClose(); }
void Window::pollEvents() { pImpl->pollEvents(); }

size_t  Window::getWidth() { return pImpl->getWidth(); }
size_t  Window::getHeight() { return pImpl->getHeight(); }

int  Window::beginRender() { return pImpl->beginRender(); }
void  Window::swapBuffers() { pImpl->swapBuffers(); }

Input::Mouse *Window::getMouse() { return pImpl->getMouse(); }
Input::Keyboard *Window::getKeyboard() { return pImpl->getKeyboard(); }

Window *Window::create(size_t width, size_t height, const char *title, int flags) {
   auto out = new Window();

   if (out->pImpl->create(width, height, title, flags)) {
      destroy(out);
      return nullptr;
   }

   return out;
}

void Window::destroy(Window *wnd) {
   if (wnd) {
      delete wnd;
   }
}

