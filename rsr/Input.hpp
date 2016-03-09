#pragma once

#include "Geom.hpp"

#include <memory>
#include <functional>

namespace Input {
   typedef enum {
      MouseBtn_Left = 0,
      MouseBtn_Right,
      MouseBtn_Middle,
      MouseBtn_4,
      MouseBtn_5,
      MouseBtn_6,
      MouseBtn_7,
      MouseBtn_8,
      MouseBtn_COUNT
   }MouseButtons;

   typedef enum {
      Mouse_Pressed = 0,
      Mouse_Released,
      Mouse_Moved,
      Mouse_Scrolled
   }MouseActions;

   typedef enum {
      Key_Pressed = 0,
      Key_Released,
      Key_Repeat,
      Key_Char
   }KeyActions;

   typedef enum {
      Key_ModShift = 1 << 0,
      Key_ModCtrl = 1 << 1,
      Key_ModAlt = 1 << 2
   }KeyMods;


   static const int Key_Undefined = -1;
   typedef enum {
      Key_Space = 0,
      Key_Apostrophe,
      Key_Comma,
      Key_Minus,
      Key_Period,
      Key_Slash,
      Key_0,
      Key_1,
      Key_2,
      Key_3,
      Key_4,
      Key_5,
      Key_6,
      Key_7,
      Key_8,
      Key_9,
      Key_Semicolon,
      Key_Equal,
      Key_A,
      Key_B,
      Key_C,
      Key_D,
      Key_E,
      Key_F,
      Key_G,
      Key_H,
      Key_I,
      Key_J,
      Key_K,
      Key_L,
      Key_M,
      Key_N,
      Key_O,
      Key_P,
      Key_Q,
      Key_R,
      Key_S,
      Key_T,
      Key_U,
      Key_V,
      Key_W,
      Key_X,
      Key_Y,
      Key_Z,
      Key_LeftBracket,
      Key_Backslash,
      Key_RightBracket,
      Key_GraveAccent,
      Key_Escape,
      Key_Enter,
      Key_Tab,
      Key_Backspace,
      Key_Insert,
      Key_Delete,
      Key_Right,
      Key_Left,
      Key_Down,
      Key_Up,
      Key_PageUp,
      Key_PageDown,
      Key_Home,
      Key_End,
      Key_CapsLock,
      Key_ScrollLock,
      Key_NumLock,
      Key_PrintScreen,
      Key_Pause,
      Key_F1,
      Key_F2,
      Key_F3,
      Key_F4,
      Key_F5,
      Key_F6,
      Key_F7,
      Key_F8,
      Key_F9,
      Key_F10,
      Key_F11,
      Key_F12,
      Key_Keypad0,
      Key_Keypad1,
      Key_Keypad2,
      Key_Keypad3,
      Key_Keypad4,
      Key_Keypad5,
      Key_Keypad6,
      Key_Keypad7,
      Key_Keypad8,
      Key_Keypad9,
      Key_KeypadDecimal,
      Key_KeypadDivide,
      Key_KeypadMultiply,
      Key_KeypadSubtract,
      Key_KeypadAdd,
      Key_KeypadEnter,
      Key_KeypadEqual,
      Key_LeftShift,
      Key_LeftControl,
      Key_LeftAlt,
      Key_RightShift,
      Key_RightControl,
      Key_RightAlt,
      Key_COUNT
   }Keys;

   /*----Mouse -------*/
   typedef struct {
      MouseActions action;
      MouseButtons button;
      ::Int2 pos;
   }MouseEvent;

   

   class Mouse {
   public:
      typedef std::function<::Int2()> PositionGet;
   private:
      class Impl;
      std::unique_ptr<Impl> pImpl;
      Mouse(PositionGet const &posGet);
      ~Mouse();
   public:
      static Mouse *create(PositionGet const &posGet);
      static void destroy(Mouse *self);

      void pushEvent(MouseEvent const &e);
      ::Int2 position();

      MouseEvent *popEvent();
      bool isDown(MouseButtons button);
      void flushQueue();
   };

   /*----Keyboard -------*/
   typedef struct {
      KeyActions action;
      Keys key;
      unsigned int unichar;
      KeyMods mods;
   }KeyboardEvent;

   class Keyboard {
      class Impl;
      std::unique_ptr<Impl> pImpl;

      Keyboard();
      ~Keyboard();
   public:
      static Keyboard *create();
      static void destroy(Keyboard *self);

      void pushEvent(KeyboardEvent const &e);

      KeyboardEvent *popEvent();
      bool isDown(Keys key);
      void flushQueue();

   };

}