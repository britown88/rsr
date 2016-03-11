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

   bool endedWithSlash = false;

   while (char c = *head) {
      if (isWhitespace(c) || c == '/') {
         if (len) {
            list.push_back(std::string(wordStart, len));
            len = 0;
         }
         else if (endedWithSlash) {
            //need to add empty string incase tex coord is skipped
            list.push_back("");
         }
         endedWithSlash = c == '/';
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
   ModelVertices v;

   FILE* file = nullptr;
   char line[256] = { 0 };
   TokenList tokens;
   bool processingFaces = false;
};

enum class LineResult : unsigned int{
   Vertex,
   Face,
   Unused,
   NewGroup,
   NewObject
};

static void addNormalIndices(OBJData &data, std::string &v1, std::string &v2, std::string &v3) {
   int norCount = data.v.normals.size();
   
   int vi1 = readInt(v1);
   if (vi1 < 0) { vi1 += norCount + 1; }
   data.v.normalIndices.push_back(vi1 - 1);

   int vi2 = readInt(v2);
   if (vi2 < 0) { vi2 += norCount + 1; }
   data.v.normalIndices.push_back(vi2 - 1);

   int vi3 = readInt(v3);
   if (vi3 < 0) { vi3 += norCount + 1; }
   data.v.normalIndices.push_back(vi3 - 1);
}
static void addPositionIndices(OBJData &data, std::string &v1, std::string &v2, std::string &v3) {
   int posCount = data.v.positions.size();
   
   int vi1 = readInt(v1);
   if (vi1 < 0) { vi1 += posCount + 1; }
   data.v.positionIndices.push_back(vi1 - 1);

   int vi2 = readInt(v2);
   if (vi2 < 0) { vi2 += posCount + 1; }
   data.v.positionIndices.push_back(vi2 - 1);

   int vi3 = readInt(v3);
   if (vi3 < 0) { vi3 += posCount + 1; }
   data.v.positionIndices.push_back(vi3 - 1);
}
static void addUVIndices(OBJData &data, std::string &uv1, std::string &uv2, std::string &uv3) {   
   if (!uv1.empty()) 
   { 
      int uvIndex = readInt(uv1);
      if (uvIndex < 0) { uvIndex = data.v.textures.size() + 1 + uvIndex; }
      data.v.textureIndices.push_back(uvIndex - 1);
   }

   if (!uv2.empty()) { 
      int uvIndex = readInt(uv2);
      if (uvIndex < 0) { uvIndex = data.v.textures.size() + 1 + uvIndex; }
      data.v.textureIndices.push_back(uvIndex - 1);
   }

   if (!uv3.empty()) { 
      int uvIndex = readInt(uv3);
      if (uvIndex < 0) { uvIndex = data.v.textures.size() + 1 + uvIndex; }
      data.v.textureIndices.push_back(uvIndex - 1);
   }
}

static LineResult processLine(TokenList &tokens, OBJData &data) {
   std::string &cmd = tokens[0];
   size_t tokenCount = tokens.size() - 1;
   
   if (cmd == "f") {
      switch (tokenCount) {
      case 3://only positions
         addPositionIndices(data, tokens[1], tokens[2], tokens[3]);
         break;
      case 6://positions and textures
         addPositionIndices(data, tokens[1], tokens[3], tokens[5]);
         addUVIndices(data,       tokens[2], tokens[4], tokens[6]);
         break;
      case 9://positions, textures, normals
         addPositionIndices(data, tokens[1], tokens[4], tokens[7]);
         addUVIndices(data,       tokens[2], tokens[5], tokens[8]);
         addNormalIndices(data,   tokens[3], tokens[6], tokens[9]);
         break;
      }

      return LineResult::Face;
   }
   else if (cmd == "v") {
      if (!data.processingFaces && tokenCount >= 3) {
         data.v.positions.push_back({ readFloat(tokens[1]), readFloat(tokens[2]), readFloat(tokens[3]) });
         //optional color
         if (tokenCount == 6) {
            data.v.colors.push_back({ readFloat(tokens[4]), readFloat(tokens[5]), readFloat(tokens[6]), 1.0f });
         }
         else if (tokenCount == 7) {
            //account for w
            data.v.colors.push_back({ readFloat(tokens[5]), readFloat(tokens[6]), readFloat(tokens[7]), 1.0f });
         }
      }      

      return LineResult::Vertex;
   }
   else if (cmd == "vt") {
      if (!data.processingFaces && tokenCount >= 2) {
         data.v.textures.push_back({ readFloat(tokens[1]), readFloat(tokens[2]) });
      }
      
      return LineResult::Vertex;
   }
   else if (cmd == "vn" ) {
      if (!data.processingFaces && tokenCount >= 3) {
         data.v.normals.push_back(vec::normal<float>({ readFloat(tokens[1]), readFloat(tokens[2]), readFloat(tokens[3]) }));
      }
      
      return LineResult::Vertex;
   }
   else if (cmd == "g") {
      return LineResult::NewGroup;
   }
   else if (cmd == "o") {
      return LineResult::NewObject;
   }   

   return LineResult::Unused;
}

std::vector<ModelVertices> ModelVertices::fromOBJ(const char *file) {
   OBJData data;
   std::vector<ModelVertices> out;

   data.file = fopen(file, "r");

   if (!data.file) {
      return out;
   }

   while (fgets(data.line, sizeof(data.line), data.file)) {
      split(data.line, data.tokens);

      bool repeat = false;
      do {
         repeat = false;
         if (!data.tokens.empty()) {
            switch (processLine(data.tokens, data)) {
            case LineResult::Face:
               data.processingFaces = true;
               break;
            case LineResult::Vertex:            
               if (data.processingFaces) {
                  if (!data.v.positionIndices.empty()) {
                     out.push_back(std::move(data.v));
                     data.v = ModelVertices();
                  }
                  data.processingFaces = false;
                  repeat = true;
               }
               break;
            case LineResult::NewObject:
               if (!data.v.positionIndices.empty()) {
                  out.push_back(data.v);
                  data.v.positionIndices.clear();
                  data.v.textureIndices.clear();
                  data.v.normalIndices.clear();
                  data.processingFaces = false;
               }
               break;
            case LineResult::NewGroup:            
               if (!data.v.positionIndices.empty()) {
                  out.push_back(std::move(data.v));
                  data.v = ModelVertices();
                  data.processingFaces = false;
               }
               break;

            default:
               break;
            }

            
         }
      } while (repeat);
   }

   if (!data.v.positionIndices.empty()) {
      out.push_back(std::move(data.v));
   }

   fclose(data.file);
   return out;
}

ModelVertices &ModelVertices::calculateNormals() {
   size_t pCount = positions.size();
   size_t piCount = positionIndices.size();

   std::vector<Float3> normalList(pCount);
   std::vector<int> normalCounts(pCount);

   for (int i = 0; i < piCount / 3; ++i) {
      int i1 = positionIndices[i * 3 + 0];
      int i2 = positionIndices[i * 3 + 1];
      int i3 = positionIndices[i * 3 + 2];

      Float3 v1 = positions[i1];
      Float3 v2 = positions[i2];
      Float3 v3 = positions[i3];

      Float3 normal = vec::normal(vec::cross(vec::sub(v2, v1), vec::sub(v3, v1)));

      normalList[i1] = vec::add(normalList[i1], normal);
      normalList[i2] = vec::add(normalList[i2], normal);
      normalList[i3] = vec::add(normalList[i3], normal);

      ++normalCounts[i1];
      ++normalCounts[i2];
      ++normalCounts[i3];
   }

   normals.clear();
   normalIndices = positionIndices;//copy

   for (int i = 0; i < pCount; ++i) {
      Float3 normal;

      if (normalCounts[i] > 0) {
         normal = vec::mul(normalList[i], 1.0f / normalCounts[i]);
      }

      normals.push_back(normal);
   }

   return *this;
}

ModelVertices &ModelVertices::expandIndices() {
   ModelVertices out;
   bool hasTextures = !textureIndices.empty();
   bool hasNormals = !normalIndices.empty();
   bool hasColors = !colors.empty();
   size_t piCount = positionIndices.size();

   //expanding vetices normalizes all 3 lists to lineup to the same size
   //no longer need for indices


   for (size_t i = 0; i < piCount; ++i) {
      int pIndex = positionIndices[i];
      out.positions.push_back(positions[pIndex]);
      if (hasColors) {
         out.colors.push_back(colors[pIndex]);
      }

      if (hasTextures) {
         int tIndex = textureIndices[i];
         out.textures.push_back(textures[tIndex]);
      }

      if (hasNormals) {
         int nIndex = normalIndices[i];
         out.normals.push_back(normals[nIndex]);
      }
   }

   *this = std::move(out);
   return *this;
}

template<typename FVF>
Model *createModelEX(ModelVertices const &vertices) {
   std::vector<FVF> outVertices;
   size_t pCount = vertices.positions.size();

   std::vector<VertexAttribute> &attrs = FVF::attrs();
   auto colorIter = std::find(attrs.begin(), attrs.end(), VertexAttribute::Col4);
   auto textureIter = std::find(attrs.begin(), attrs.end(), VertexAttribute::Tex2);
   auto normalIter = std::find(attrs.begin(), attrs.end(), VertexAttribute::Norm3);

   bool hasColor = colorIter != attrs.end();
   bool hasTexture = textureIter != attrs.end();
   bool hasNormal = normalIter != attrs.end();

   int colorPos = 0, texturePos = 0, normalPos = 0;

   if (hasColor) { colorPos = colorIter - attrs.begin(); }
   if (hasTexture) { texturePos = textureIter - attrs.begin(); }
   if (hasNormal) { normalPos = normalIter - attrs.begin(); }

   std::vector<int> attrOffsets(attrs.size());
   int totalSize = 0;
   for (int i = 0; i < attrs.size(); ++i) {
      attrOffsets[i] = totalSize;
      totalSize += vertexAttributeByteSize(attrs[i]);
      
   }

   for (size_t i = 0; i < pCount; ++i) {
      FVF vertex;
      vertex.pos3 = vertices.positions[i];

      //cant specify the names of the attrs directly
      //have to do some c-style byte-casting with offsets to get the data into the right spot
      if (hasColor) { *(ColorRGBAf*)(((unsigned char *)&vertex) + attrOffsets[colorPos]) = vertices.colors.empty() ? CommonColors::White : vertices.colors[i]; }
      if (hasTexture) { *(Float2*)(((unsigned char *)&vertex) + attrOffsets[texturePos]) = vertices.textures[i]; }
      if (hasNormal) { *(Float3*)(((unsigned char *)&vertex) + attrOffsets[normalPos]) = vertices.normals[i]; }

      outVertices.push_back(vertex);
   }

   return ModelManager::create(outVertices);
}

Model *ModelVertices::createModel(int modelOptions) {
   bool c = modelOptions&ModelOpts::IncludeColor;
   bool t = modelOptions&ModelOpts::IncludeTexture;
   bool n = modelOptions&ModelOpts::IncludeNormals;

   if (c && t && n) { return createModelEX<FVF_Pos3_Norm3_Tex2_Col4>(*this); }
   else if (c && n) { return createModelEX<FVF_Pos3_Norm3_Col4>(*this); }
   else if(c && t ) { return createModelEX<FVF_Pos3_Tex2_Col4>(*this); }
   else if(t && n) { return createModelEX<FVF_Pos3_Norm3_Tex2>(*this); }
   else if(c) { return createModelEX<FVF_Pos3_Col4>(*this); }
   else if(t) { return createModelEX<FVF_Pos3_Tex2>(*this); }
   else if(n) { return createModelEX<FVF_Pos3_Norm3>(*this); }
   else { return createModelEX<FVF_Pos3>(*this); }
}