#include "Model.hpp"

#include <string>
#include <vector>

static bool isWhitespace(char c) {
   return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

typedef std::vector < std::string > TokenList;

static void split(const char *line, TokenList &list) {
   list.clear();
   
   char *head = (char*)line;

   char *wordStart = head;
   int len = 0;

   while (char c = *head) {
      if (isWhitespace(c) || c == '/') {
         if (len) {
            list.push_back(std::string(wordStart, len));
            len = 0;
         }
      }
      else {
         if (len == 0) {
            wordStart = head;
         }
         ++len;
      }

      ++head;
   }

   if (len) {
      list.push_back(std::string(wordStart, len));
   }
}

static float readFloat(std::string &str) {
   float f;
   if (sscanf(str.c_str(), "%f", &f) > 0) {
      return f;
   }
   return 0.0f;
}

static int readInt(std::string &str) {
   int i;
   if (sscanf(str.c_str(), "%i", &i) > 0) {
      return i;
   }
   return 0;
}

struct OBJData {
   std::vector<Float3> positions;
   std::vector<Float2> textures;
   std::vector<Float3> normals;
   std::vector<ColorRGBAf> colors;

   bool faceSetup = false;
   bool hasColor = false;
   FILE* file;

   char line[256];
   TokenList tokens;
};

static void read(TokenList &tokens, OBJData &data) {
   if (tokens[0] == "v" && tokens.size() >= 4) {
      data.positions.push_back({ readFloat(tokens[1]), readFloat(tokens[2]), readFloat(tokens[3])});
      if (tokens.size() >= 7) {
         data.colors.push_back({ readFloat(tokens[4]), readFloat(tokens[5]), readFloat(tokens[6]), 1.0f });
      }
   }
   else if (tokens[0] == "vt" && tokens.size() >= 3) {
      data.textures.push_back({ readFloat(tokens[1]), readFloat(tokens[2])});
   }
   else if (tokens[0] == "vn" && tokens.size() >= 4) {
      data.normals.push_back({ readFloat(tokens[1]), readFloat(tokens[2]), readFloat(tokens[3]) });
   }
   else if (tokens[0] == "f") {
      data.faceSetup = true;
   }
}

Model *buildPosCol(OBJData &data) {
   std::vector<FVF_Pos3_Col4> vertices;
   std::vector<int> indices;
   ColorRGBAf white = { 1.0f, 1.0f, 1.0f, 1.0f };

   int index = 0;
   for(auto &pos : data.positions) {
      vertices.push_back({ pos, data.hasColor ? data.colors[index++] : white });
   }

   int vIndex = 0;
   while (fgets(data.line, sizeof(data.line), data.file)) {
      split(data.line, data.tokens);
      if (data.tokens.size() == 4 && data.tokens[0] == "f") {
         for (int i = 0; i < 3; ++i) {
            int index = readInt(data.tokens[1 + i]) - 1;
            indices.push_back(index);
         }
      }
   }
   return ModelManager::create( vertices.data(), vertices.size(), indices.data(), indices.size());
}

Model *buildPosTexCol(OBJData &data) {
   std::vector<FVF_Pos3_Tex2_Col4> vertices;
   std::vector<int> indices;
   return ModelManager::create(vertices.data(), vertices.size(), indices.data(), indices.size());
}

Model *buildPosNormTexCol(OBJData &data) {
   std::vector<FVF_Pos3_Norm3_Tex2_Col4> vertices;
   std::vector<int> indices;
   return ModelManager::create(vertices.data(), vertices.size(), indices.data(), indices.size());
}

Model *buildPosNormCol(OBJData &data) {
   std::vector<FVF_Pos3_Norm3_Col4> vertices;
   std::vector<int> indices;
   return ModelManager::create(vertices.data(), vertices.size(), indices.data(), indices.size());
}

Model *ModelManager::importFromOBJ(const char *file) {
   OBJData data;

   data.file = fopen(file, "r");

   if (!data.file) {
      return nullptr;
   }

   //vertex lines
   while (fgets(data.line, sizeof(data.line), data.file)) {
      split(data.line, data.tokens);
      if (!data.tokens.empty()) {
         read(data.tokens, data);
         if (data.faceSetup) {
            break;
         }
      }
   }

   data.hasColor = !data.colors.empty();
   bool hasTexture = !data.textures.empty();
   bool hasNormal = !data.normals.empty();

   Model *out = nullptr;

   if (hasTexture && hasNormal) {
      out = buildPosNormTexCol(data);
   }
   else if (hasTexture) {
      out = buildPosTexCol(data);
   }
   else if (hasNormal){
      out = buildPosNormCol(data);
   }
   else {
      out = buildPosCol(data);
   }

   fclose(data.file);
   return out;   
}