#pragma once

#include "Window.hpp"

#include <windows.h>
#include <windowsx.h>
#include <GL/GL.h>
#include <GL/GLU.h>

#include <functional>



const char g_szClassName[] = "rsrWin";

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static Window::Impl *Instance;

class Window::Impl {
public:   

   HWND m_hWnd = NULL;
   HDC m_hdc = NULL;
   HGLRC m_hContext = NULL;
   HINSTANCE m_hInstance = NULL;
   size_t m_width = 0, m_height = 0;
   bool m_shouldClose = false;

   Keyboard *m_keyboard;
   Mouse *m_mouse;

   Int2 getMousePosition() {
      POINT p;
      GetCursorPos(&p);
      ScreenToClient(m_hWnd, &p);

      return{ (int)p.x, (int)p.y };
   }

   ~Impl() {
      wglMakeCurrent(NULL, NULL);
      wglDeleteContext(m_hContext);
      ReleaseDC(m_hWnd, m_hdc);

      Mouse::destroy(m_mouse);
      Keyboard::destroy(m_keyboard);
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

      m_keyboard = Keyboard::create();
      m_mouse = Mouse::create([=]() {return getMousePosition();});
      
      Instance = (Window::Impl*)this;

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

   Mouse *getMouse() { return m_mouse; }
   Keyboard *getKeyboard() { return m_keyboard; }

   void close() {
      m_shouldClose = true;
   }

   void setCapture(bool capture) {
      if (capture) {
         SetCapture(m_hWnd);
      }
      else {
         ReleaseCapture();
      }
   }
};

void inputChar(unsigned int c) {
   KeyboardEvent e = {
      KeyActions::Key_Char,
      (Keys)0, c};

   Instance->getKeyboard()->pushEvent(e);
}

Keys getKeyFromWindows(WPARAM code) {
   static const int keyCount = VK_OEM_CLEAR + 1;
   static Keys map[keyCount];
   static bool mapInit = false;

   if (!mapInit) {
      mapInit = true;

      for (int i = 0; i < keyCount; ++i) {
         map[i] = (Keys)Key_Undefined;
      }

      map[VK_SPACE] = Key_Space;
      map[VK_OEM_7] = Key_Apostrophe;
      map[VK_OEM_COMMA] = Key_Comma;
      map[VK_OEM_MINUS] = Key_Minus;
      map[VK_OEM_PERIOD] = Key_Period;
      map[VK_OEM_2] = Key_Slash;
      map['0'] = Key_0;
      map['1'] = Key_1;
      map['2'] = Key_2;
      map['3'] = Key_3;
      map['4'] = Key_4;
      map['5'] = Key_5;
      map['6'] = Key_6;
      map['7'] = Key_7;
      map['8'] = Key_8;
      map['9'] = Key_9;
      map[VK_OEM_1] = Key_Semicolon;
      map[VK_OEM_PLUS] = Key_Equal;
      map['A'] = Key_A;
      map['B'] = Key_B;
      map['C'] = Key_C;
      map['D'] = Key_D;
      map['E'] = Key_E;
      map['F'] = Key_F;
      map['G'] = Key_G;
      map['H'] = Key_H;
      map['I'] = Key_I;
      map['J'] = Key_J;
      map['K'] = Key_K;
      map['L'] = Key_L;
      map['M'] = Key_M;
      map['N'] = Key_N;
      map['O'] = Key_O;
      map['P'] = Key_P;
      map['Q'] = Key_Q;
      map['R'] = Key_R;
      map['S'] = Key_S;
      map['T'] = Key_T;
      map['U'] = Key_U;
      map['V'] = Key_V;
      map['W'] = Key_W;
      map['X'] = Key_X;
      map['Y'] = Key_Y;
      map['Z'] = Key_Z;
      map[VK_OEM_4] = Key_LeftBracket;
      map[VK_OEM_5] = Key_Backslash;
      map[VK_OEM_6] = Key_RightBracket;
      map[VK_OEM_3] = Key_GraveAccent;
      map[VK_ESCAPE] = Key_Escape;
      map[VK_RETURN] = Key_Enter;
      map[VK_TAB] = Key_Tab;
      map[VK_BACK] = Key_Backspace;
      map[VK_INSERT] = Key_Insert;
      map[VK_DELETE] = Key_Delete;
      map[VK_RIGHT] = Key_Right;
      map[VK_LEFT] = Key_Left;
      map[VK_DOWN] = Key_Down;
      map[VK_UP] = Key_Up;
      map[VK_PRIOR] = Key_PageUp;
      map[VK_NEXT] = Key_PageDown;
      map[VK_HOME] = Key_Home;
      map[VK_END] = Key_End;
      map[VK_CAPITAL] = Key_CapsLock;
      map[VK_SCROLL] = Key_ScrollLock;
      map[VK_NUMLOCK] = Key_NumLock;
      map[VK_SNAPSHOT] = Key_PrintScreen;
      map[VK_PAUSE] = Key_Pause;
      map[VK_F1] = Key_F1;
      map[VK_F2] = Key_F2;
      map[VK_F3] = Key_F3;
      map[VK_F4] = Key_F4;
      map[VK_F5] = Key_F5;
      map[VK_F6] = Key_F6;
      map[VK_F7] = Key_F7;
      map[VK_F8] = Key_F8;
      map[VK_F9] = Key_F9;
      map[VK_F10] = Key_F10;
      map[VK_F11] = Key_F11;
      map[VK_F12] = Key_F12;
      map[VK_NUMPAD0] = Key_Keypad0;
      map[VK_NUMPAD1] = Key_Keypad1;
      map[VK_NUMPAD2] = Key_Keypad2;
      map[VK_NUMPAD3] = Key_Keypad3;
      map[VK_NUMPAD4] = Key_Keypad4;
      map[VK_NUMPAD5] = Key_Keypad5;
      map[VK_NUMPAD6] = Key_Keypad6;
      map[VK_NUMPAD7] = Key_Keypad7;
      map[VK_NUMPAD8] = Key_Keypad8;
      map[VK_NUMPAD9] = Key_Keypad9;
      map[VK_DECIMAL] = Key_KeypadDecimal;
      map[VK_DIVIDE] = Key_KeypadDivide;
      map[VK_NUMPAD9] = Key_KeypadMultiply;
      map[VK_SUBTRACT] = Key_KeypadSubtract;
      map[VK_ADD] = Key_KeypadAdd;
      map[VK_RETURN] = Key_KeypadEnter;
      map[VK_OEM_NEC_EQUAL] = Key_KeypadEqual;
      map[VK_LSHIFT] = Key_LeftShift;
      map[VK_LCONTROL] = Key_LeftControl;
      map[VK_LMENU] = Key_LeftAlt;
      map[VK_RSHIFT] = Key_RightShift;
      map[VK_RCONTROL] = Key_RightControl;
      map[VK_RMENU] = Key_RightAlt;
   }

   return map[code];
}

void inputKey(KeyActions action, WPARAM code) {
   KeyboardEvent e = { action, getKeyFromWindows(code) };

   if (e.key != (Keys)Key_Undefined) {
      Instance->getKeyboard()->pushEvent(e);
   }
}

void inputMouse(MouseActions action, MouseButtons button, short x, short y) {
   MouseEvent me = { action, button, {(int)x, (int)y} };

   if (action == MouseActions::Mouse_Pressed) {
      Instance->setCapture(true);
   }
   else if (action == MouseActions::Mouse_Released) {
      Instance->setCapture(false);
   }

   Instance->getMouse()->pushEvent(me);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

   static Int2 lastMousePos = { 0 };
   switch (msg)
   {
   case WM_CLOSE:
      DestroyWindow(hwnd);
      break;
   case WM_DESTROY:
      PostQuitMessage(0);
      break;

   case WM_CHAR:
      inputChar((char)wParam);
      break;
   case WM_KEYDOWN:
      inputKey(KeyActions::Key_Pressed, wParam);
      break;
   case WM_KEYUP:
      inputKey(KeyActions::Key_Released, wParam);
      break;

   case WM_MOUSEMOVE:
      inputMouse(MouseActions::Mouse_Moved, MouseButtons::MouseBtn_COUNT, 
         GET_X_LPARAM(lParam) - lastMousePos.x, 
         GET_Y_LPARAM(lParam) - lastMousePos.y);
      lastMousePos = { GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) };
      break;
   case WM_MOUSEWHEEL:
      inputMouse(MouseActions::Mouse_Scrolled, MouseButtons::MouseBtn_Middle, 0, HIWORD(wParam));
      break;
   case WM_LBUTTONUP:
      inputMouse(MouseActions::Mouse_Released, MouseButtons::MouseBtn_Left, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      break;
   case WM_LBUTTONDOWN:
      inputMouse(MouseActions::Mouse_Pressed, MouseButtons::MouseBtn_Left, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      break;
   case WM_LBUTTONDBLCLK:
      inputMouse(MouseActions::Mouse_DblClicked, MouseButtons::MouseBtn_Left, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      break;
   case WM_RBUTTONUP:
      inputMouse(MouseActions::Mouse_Released, MouseButtons::MouseBtn_Right, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      break;
   case WM_RBUTTONDOWN:
      inputMouse(MouseActions::Mouse_Pressed, MouseButtons::MouseBtn_Right, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      break;
   case WM_RBUTTONDBLCLK:
      inputMouse(MouseActions::Mouse_DblClicked, MouseButtons::MouseBtn_Right, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      break;
   case WM_MBUTTONUP:
      inputMouse(MouseActions::Mouse_Released, MouseButtons::MouseBtn_Middle, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      break;
   case WM_MBUTTONDOWN:
      inputMouse(MouseActions::Mouse_Pressed, MouseButtons::MouseBtn_Middle, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      break;
   case WM_MBUTTONDBLCLK:
      inputMouse(MouseActions::Mouse_DblClicked, MouseButtons::MouseBtn_Middle, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      break;

   default:
      return DefWindowProc(hwnd, msg, wParam, lParam);
   }
   return 0;
}


Window::Window() :pImpl(new Impl()) { }
Window::~Window() {}

bool Window::shouldClose() { return pImpl->shouldClose(); }
void Window::pollEvents() { pImpl->pollEvents(); }

size_t  Window::getWidth() { return pImpl->getWidth(); }
size_t  Window::getHeight() { return pImpl->getHeight(); }

int  Window::beginRender() { return pImpl->beginRender(); }
void  Window::swapBuffers() { pImpl->swapBuffers(); }

Mouse *Window::getMouse() { return pImpl->getMouse(); }
Keyboard *Window::getKeyboard() { return pImpl->getKeyboard(); }

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

void Window::close() {
   pImpl->close();
}
