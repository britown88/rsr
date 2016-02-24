#include "StringView.hpp"
#include "Singleton.hpp"
#include <unordered_set>
#include <string>
#include <string.h>

struct StringHash {
   size_t operator()(StringView view) const;
};

struct StringComp {
   bool operator()(StringView lhs, StringView rhs) const;
};

class StringTable {
   std::unordered_set<StringView, StringHash, StringComp> m_table;
public:
   ~StringTable();

   StringView get(const char* str);
};

size_t StringHash::operator()(StringView view) const {
   size_t out = 5381;
   const char *str = (const char *)view;
   while (*str)
      out = (out << 5) + (out << 1) + (*str++);

   return out;
}

bool StringComp::operator()(StringView lhs, StringView rhs) const {
   return strcmp((const char *)lhs, (const char *)rhs) == 0;
}

StringTable::~StringTable() {
   //destruction should be at porogram termination but let's class this joint up
   for (auto &&item : m_table) {
      free((void*)item);
   }
}

StringView StringTable::get(const char* str) {
   auto found = m_table.find((StringView)str);
   if (found == m_table.end()) {
      auto len = strlen(str) + 1;
      StringView viewStorage = (StringView)malloc(len);
      memcpy((char*)viewStorage, str, len);
      found = m_table.insert(viewStorage).first;
   }

   return *found;
}

StringView internString(const char* str) {
   return Singleton<StringTable>::Instance().get(str);
}

size_t stringViewHash(StringView str) {
   static std::hash<void*> hashFunc;
   return hashFunc((void*)str);
}
