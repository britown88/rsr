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
   bool finished = false;
   bool hasColor = false;
   FILE* file;

   char line[256];
   TokenList tokens;
   int vIndex = 0;
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

template<typename T>
void calculateNormals(std::vector<T> &vertices, std::vector<int> &indices, std::vector<Float3> &normals, std::vector<int> &normalCounts) {
   for (int i = 0; i < indices.size() / 3; ++i) {
      int i1 = indices[i * 3 + 0];
      int i2 = indices[i * 3 + 1];
      int i3 = indices[i * 3 + 2];

      Float3 v1 = vertices[i1].pos3;
      Float3 v2 = vertices[i2].pos3;
      Float3 v3 = vertices[i3].pos3;

      Float3 normal = vec::normal(vec::cross(vec::sub(v2, v1), vec::sub(v3, v1)));

      normals[i1] = vec::add(normals[i1], normal);
      normals[i2] = vec::add(normals[i2], normal);
      normals[i3] = vec::add(normals[i3], normal);

      ++normalCounts[i1];
      ++normalCounts[i2];
      ++normalCounts[i3];
   }
}

void addPosCol(OBJData &data, std::vector<FVF_Pos3_Col4> &vertices, std::vector<int> &indices) {
   if (!data.tokens.empty() && data.tokens[0] == "g") {
      data.finished = true;
      return;
   }
   
   if (data.tokens.size() == 4 && data.tokens[0] == "f") {
      for (int i = 0; i < 3; ++i) {
         int index = readInt(data.tokens[1 + i]) - 1;
         indices.push_back(index);
      }
   }
}
Model *buildPosCol(OBJData &data) {
   std::vector<FVF_Pos3_Col4> vertices;
   std::vector<int> indices;

   int index = 0;
   for(auto &pos : data.positions) {
      vertices.push_back({ pos, data.hasColor ? data.colors[index++] : CommonColors::White });
   }

   addPosCol(data, vertices, indices);
   while (fgets(data.line, sizeof(data.line), data.file)) {
      split(data.line, data.tokens);
      addPosCol(data, vertices, indices);
      if (data.finished) {
         break;
      }
   }

   std::vector<Float3> normals(vertices.size());;
   std::vector<int> normalCounts(vertices.size());

   calculateNormals(vertices, indices, normals, normalCounts);

   std::vector<FVF_Pos3_Norm3_Col4> normVertices;
   for (int i = 0; i < vertices.size(); ++i) {
      FVF_Pos3_Col4 &old = vertices[i];

      Float3 normal;

      if (normalCounts[i] > 0) {
         normal = vec::mul(normals[i], 1.0f / normalCounts[i]);
      }

      normVertices.push_back({ old.pos3, normal, old.col4 });
   }

   return ModelManager::create(normVertices.data(), normVertices.size(), indices.data(), indices.size());
}

void addPosTexCol(OBJData &data, std::vector<FVF_Pos3_Tex2_Col4> &vertices, std::vector<int> &indices) {   
   if (!data.tokens.empty() && data.tokens[0] == "g") {
      data.finished = true;
      return;
   }
   if (data.tokens.size() == 7 && data.tokens[0] == "f") {
      for (int i = 0; i < 3; ++i) {
         int pindex = readInt(data.tokens[1 + i*2]) - 1;
         int tindex = readInt(data.tokens[2 + i*2]) - 1;
         
         vertices.push_back({ data.positions[pindex], data.textures[tindex], data.hasColor ? data.colors[pindex] : CommonColors::White });
         indices.push_back(data.vIndex++);
      }
   }
}
Model *buildPosTexCol(OBJData &data) {
   std::vector<FVF_Pos3_Tex2_Col4> vertices;
   std::vector<int> indices;

   addPosTexCol(data, vertices, indices);
   while (fgets(data.line, sizeof(data.line), data.file)) {
      split(data.line, data.tokens);
      addPosTexCol(data, vertices, indices);
      if (data.finished) {
         break;
      }
   }

   std::vector<Float3> normals(vertices.size());;
   std::vector<int> normalCounts(vertices.size());

   calculateNormals(vertices, indices, normals, normalCounts);

   std::vector<FVF_Pos3_Norm3_Tex2_Col4> normVertices;
   for (int i = 0; i < vertices.size(); ++i) {
      FVF_Pos3_Tex2_Col4 &old = vertices[i];

      Float3 normal;

      if (normalCounts[i] > 0) {
         normal = vec::mul(normals[i], 1.0f / normalCounts[i]);
      }

      normVertices.push_back({old.pos3, normal, old.tex2, old.col4});
   }

   return ModelManager::create(normVertices.data(), normVertices.size(), indices.data(), indices.size());
}

void addPosNormTexCol(OBJData &data, std::vector<FVF_Pos3_Norm3_Tex2_Col4> &vertices, std::vector<int> &indices) {
   if (!data.tokens.empty() && data.tokens[0] == "g") {
      data.finished = true;
      return;
   }
   if (data.tokens.size() == 10 && data.tokens[0] == "f") {
      for (int i = 0; i < 3; ++i) {
         int pindex = readInt(data.tokens[1 + i * 3]);
         int tindex = readInt(data.tokens[2 + i * 3]);
         int nindex = readInt(data.tokens[3 + i * 3]);

         if (pindex < 0) { pindex = data.positions.size() + 1 + pindex; }
         if (tindex < 0) { tindex = data.normals.size() + 1 + tindex; }
         if (nindex < 0) { nindex = data.textures.size() + 1 + nindex; }

         pindex -= 1;
         tindex -= 1;
         nindex -= 1;

         vertices.push_back({ data.positions[pindex], data.normals[nindex], data.textures[tindex], data.hasColor ? data.colors[pindex] : CommonColors::White });
         indices.push_back(data.vIndex++);
      }
   }
}
Model *buildPosNormTexCol(OBJData &data) {
   std::vector<FVF_Pos3_Norm3_Tex2_Col4> vertices;
   std::vector<int> indices;

   addPosNormTexCol(data, vertices, indices);
   while (fgets(data.line, sizeof(data.line), data.file)) {
      split(data.line, data.tokens);
      addPosNormTexCol(data, vertices, indices);
      if (data.finished) {
         break;
      }
   }

   return ModelManager::create(vertices.data(), vertices.size(), indices.data(), indices.size());
}

void addPosNormCol(OBJData &data, std::vector<FVF_Pos3_Norm3_Col4> &vertices, std::vector<int> &indices) {
   if (!data.tokens.empty() && data.tokens[0] == "g") {
      data.finished = true;
      return;
   }
   if (data.tokens.size() == 7 && data.tokens[0] == "f") {
      for (int i = 0; i < 3; ++i) {
         int pindex = readInt(data.tokens[1 + i * 2]) - 1;
         int nindex = readInt(data.tokens[2 + i * 2]) - 1;

         vertices.push_back({ data.positions[pindex], data.normals[nindex], data.hasColor ? data.colors[pindex] : CommonColors::White });
         indices.push_back(data.vIndex++);
      }
   }
}
Model *buildPosNormCol(OBJData &data) {
   std::vector<FVF_Pos3_Norm3_Col4> vertices;
   std::vector<int> indices;

   addPosNormCol(data, vertices, indices);
   while (fgets(data.line, sizeof(data.line), data.file)) {
      split(data.line, data.tokens);
      addPosNormCol(data, vertices, indices);
      if (data.finished) {
         break;
      }
   }

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