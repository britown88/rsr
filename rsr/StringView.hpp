#pragma once

struct StringViewSlot {
   char name[4];
};
typedef StringViewSlot const* StringView;

StringView internString(const char* str);
size_t stringViewHash(StringView str);