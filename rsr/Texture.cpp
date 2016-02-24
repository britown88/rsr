#include "GL/glew.h"

#include "Texture.hpp"
#include "libpng/png.h"

#include "Color.hpp"
#include "Geom.hpp"

#include "Singleton.hpp"

#include <algorithm>
#include <memory>
#include <unordered_map>


TextureRequest::TextureRequest(StringView path, RepeatType repeat, FilterType filter)
   :path(path), repeatType(repeat), filterType(filter) {}

bool TextureRequest::operator==(const TextureRequest &rhs)const {
   return repeatType == rhs.repeatType && filterType == rhs.filterType && path == rhs.path;
}

size_t TextureRequest::hash() const {
   static size_t h = 5381;
   h = (h << 5) + (h << 1) + (unsigned int)repeatType;
   h = (h << 5) + (h << 1) + (unsigned int)filterType;
   h = (h << 5) + (h << 1) + (unsigned int)stringViewHash(path);
   return h;
}

struct TextureBuffer {
   std::unique_ptr<ColorRGBA[]> bits;
   Int2 size;

   TextureBuffer() {}

   TextureBuffer(std::unique_ptr<ColorRGBA[]> bits, Int2 size)
      :bits(std::move(bits)), size(size) {}

   TextureBuffer(TextureBuffer && rhs)
      :bits(std::move(rhs.bits)), size(rhs.size) {}
   TextureBuffer &operator=(TextureBuffer && rhs) {
      bits = std::move(rhs.bits);
      size = rhs.size;
      return *this;
   }
};

TextureBuffer loadPng(std::string const& textureFile) {
   FILE* infile = fopen(textureFile.c_str(), "rb");
   if (!infile) {
      throw std::exception("failed to load texture.");
   }

   unsigned char sig[8];
   fread(sig, 1, 8, infile);
   if (!png_check_sig(sig, 8)) {
      fclose(infile);
      throw std::exception("failed to load texture.");
   }

   png_structp png_ptr;
   png_infop info_ptr;
   png_infop end_ptr;

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png_ptr) {
      fclose(infile);
      throw std::exception("failed to load texture.");
   }

   info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr) {
      png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
      fclose(infile);
      throw std::exception("failed to load texture.");
   }

   end_ptr = png_create_info_struct(png_ptr);
   if (!end_ptr) {
      png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
      fclose(infile);
      throw std::exception("failed to load texture.");
   }

   if (setjmp(png_jmpbuf(png_ptr))) {
      png_destroy_read_struct(&png_ptr, &info_ptr, &end_ptr);
      fclose(infile);
      throw std::exception("failed to load texture.");
   }

   png_ptr->io_ptr = (png_voidp)infile;
   png_set_sig_bytes(png_ptr, 8);


   int  bit_depth;
   int  color_type;

   unsigned long width;
   unsigned long height;
   unsigned int rowbytes;

   png_read_info(png_ptr, info_ptr);
   png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
      &color_type, NULL, NULL, NULL);

   png_read_update_info(png_ptr, info_ptr);

   if (bit_depth > 8) {
      png_set_strip_16(png_ptr);
   }
   if (color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
      png_set_gray_to_rgb(png_ptr);
   }
   if (color_type == PNG_COLOR_TYPE_PALETTE) {
      png_set_palette_to_rgb(png_ptr);
   }
   rowbytes = png_get_rowbytes(png_ptr, info_ptr);

   ColorRGBA* image_data;
   png_bytepp row_pointers = NULL;

   size_t totalSize = rowbytes*height;
   bool noAlpha = (rowbytes / width) == 3;
   if (noAlpha) {
      totalSize = (totalSize * 4) / 3;
   }

   if ((image_data = new ColorRGBA[totalSize / 4]) == NULL) {
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      throw std::exception("failed to load texture.");
   }

   if ((row_pointers = (png_bytepp)malloc(height*sizeof(png_bytep))) == NULL) {
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      free(image_data);
      image_data = NULL;
      throw std::exception("failed to load texture.");
   }

   for (unsigned int i = 0; i < height; ++i) {
      row_pointers[i] = (unsigned char*)(image_data)+i*(rowbytes);
   }

   png_read_image(png_ptr, row_pointers);

   free(row_pointers);
   png_destroy_read_struct(&png_ptr, &info_ptr, &end_ptr);
   fclose(infile);

   if (noAlpha) {
      unsigned char* actual = ((unsigned char*)image_data) + totalSize - 1;
      unsigned char* original = ((unsigned char*)image_data) + ((totalSize / 4) * 3) - 1;
      for (unsigned int i = 0; i < width * height; ++i) {
         *actual-- = 255;
         *actual-- = *original--;
         *actual-- = *original--;
         *actual-- = *original--;
      }
   }

   TextureBuffer out = { std::unique_ptr<ColorRGBA[]>(image_data),{ (int)width, (int)height } };
   return std::move(out);
}

class Texture {
   bool m_isLoaded;
   const TextureRequest m_request;
   GLuint m_glHandle;
   TextureBuffer m_buffer;
public:
   Texture(TextureRequest const &request) :m_isLoaded(false), m_glHandle(-1), m_request(request) {}

   void acquire() {
      if (!m_request.path)
         return;

      m_buffer = std::move(loadPng((const char*)m_request.path));

      glEnable(GL_TEXTURE_2D);
      glGenTextures(1, &m_glHandle);
      glBindTexture(GL_TEXTURE_2D, m_glHandle);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      switch (m_request.filterType)
      {
      case FilterType::Linear:
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
         break;
      case FilterType::Nearest:
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
         break;
      };

      switch (m_request.repeatType)
      {
      case RepeatType::Repeat:
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
         break;
      case RepeatType::Clamp:
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
         break;
      };

      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_buffer.size.x, m_buffer.size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_buffer.bits.get());

      glBindTexture(GL_TEXTURE_2D, 0);

      m_isLoaded = true;
   }
   void release() {
      glDeleteTextures(1, &m_glHandle);
      m_buffer.bits.reset();

      m_glHandle = 0;
      m_isLoaded = false;
   }
   bool isLoaded() { return m_isLoaded; }

   GLuint getHandle() { return m_glHandle; }
};

template<typename T>
class ObjectHash
{
public:
   size_t operator()(T const &obj) const { return obj.hash(); }
};

class TextureManagerPrivate {
   mutable std::unordered_map<TextureRequest, std::unique_ptr<Texture>, ObjectHash<TextureRequest>> m_textures;
public:
   TextureManagerPrivate() {}
   Texture *get(TextureRequest const &request) const {
      auto found = m_textures.find(request);
      if (found == m_textures.end()) {
         found = m_textures.insert(std::make_pair(request, std::unique_ptr<Texture>(new Texture(request)))).first;
      }

      return found->second.get();
   }
};
typedef Singleton<TextureManagerPrivate> inner;

Texture *TextureManager::get(TextureRequest const &request) { return inner::Instance().get(request); }

void TextureManager::bind(Texture *self, TextureSlot slot) {
   if (!self->isLoaded()) {
      self->acquire();
   }

   glActiveTexture(GL_TEXTURE0 + slot);
   glBindTexture(GL_TEXTURE_2D, self->getHandle());
}
