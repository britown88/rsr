#pragma once

#include <memory>
#include <vector>

#include "Defs.hpp"
#include "Singleton.hpp"


class DrawQueue {
   const static size_t PageSize = 4096;
   struct Page {
      byte data[PageSize];
      size_t size;
   };

   class ICall {
   public:
      virtual size_t size() = 0;
      virtual void destroy(void *buffer) = 0;
      virtual void execute(void *buffer) = 0;
   };

   template<typename T>
   class Call : public ICall {
   public:
      static const size_t m_size = sizeof(Call*) + sizeof(T);
      static const size_t m_padding = m_size % sizeof(void*);
      static const size_t m_paddedSize = m_size + (m_padding ? sizeof(void*) - m_padding : 0);

      size_t size() {
         return m_paddedSize;
      }

      void destroy(void *buffer) {
         ((T*)buffer)->~T();
      }
      void execute(void *buffer) {
         ((T*)buffer)->operator()();
      }
   };

   std::vector<Page> m_pages;

public:
   DrawQueue() {
   }
   ~DrawQueue() {
      for (auto && p : m_pages) {
         size_t i = 0;
         while (i < p.size) {
            ICall *c = *(ICall**)(p.data + i);

            c->destroy((void*)(p.data + i + sizeof(ICall*)));
            i += c->size();
         }
      }
   }

   template<typename L>
   void push(L && lambda) {

      ICall *call = &Singleton<Call<typename std::decay<L>::type>>::Instance();

      size_t callSize = call->size();

      if (callSize > PageSize) {
         return;
      }

      if (m_pages.empty() || m_pages.back().size + callSize > PageSize) {
         m_pages.push_back({ 0 });
      }

      auto &p = m_pages.back();

      *(ICall**)(p.data + p.size) = call;
      new(p.data + p.size + sizeof(ICall*)) L(std::forward<L>(lambda));

      p.size += callSize;
   }

   void draw() {
      for (auto && p : m_pages) {
         size_t i = 0;
         while (i < p.size) {
            ICall *c = *(ICall**)(p.data + i);

            c->execute((void*)(p.data + i + sizeof(ICall*)));
            i += c->size();
         }
      }
   }
};

