#include "Track.hpp"

void calculateNormals(std::vector<Float3> &vertices, std::vector<int> &indices, std::vector<Float3> &oNormals, std::vector<int> &oNormalCounts) {

   for (int i = 0; i < indices.size() / 3; ++i) {
      int i1 = indices[i * 3 + 0];
      int i2 = indices[i * 3 + 1];
      int i3 = indices[i * 3 + 2];

      Float3 v1 = vertices[i1];
      Float3 v2 = vertices[i2];
      Float3 v3 = vertices[i3];

      Float3 normal = vec::normal(vec::cross(vec::sub(v2, v1), vec::sub(v3, v1)));

      oNormals[i1] = vec::add(oNormals[i1], normal);
      oNormals[i2] = vec::add(oNormals[i2], normal);
      oNormals[i3] = vec::add(oNormals[i3], normal);

      ++oNormalCounts[i1];
      ++oNormalCounts[i2];
      ++oNormalCounts[i3];
   }
}

void calculateSingleEdgePair(TrackPoint &p1, TrackPoint &p2, TrackPoint &p3, TrackPoint &p4, float t, Float3 &oLeftPoint, Float3 &oRightPoint) {

   auto c1 = vec::cinterp(p1.pos, p2.pos, p3.pos, p4.pos, t);
   auto c2 = vec::cinterp(p1.pos, p2.pos, p3.pos, p4.pos, t + 0.01f);

   Float3 dir = vec::normal(vec::sub(c2, c1));
   Float3 up = { 0.0f, 1.0f, 0.0f };
   Float3 left = vec::cross(dir, up);

   float roll = cinterp(p1.roll, p2.roll, p3.roll, p4.roll, t);
   float width = cinterp(p1.width, p2.width, p3.width, p4.width, t);

   left = Quaternion::fromAxisAngle(dir, roll).rotate(left);
   left = vec::mul(vec::normal(left), width);
   

   Float3 right = vec::negate(left);
   oLeftPoint = vec::add(c1, left);
   oRightPoint = vec::add(c1, right);
}




void computeEdges(TrackPoint &p1, TrackPoint &p2, TrackPoint &p3, TrackPoint &p4, std::vector<Float3> &leftList, std::vector<Float3> &rightList) {
   static const int passCount = 10;

   for (int j = 0; j < passCount; ++j) {
      float t = (float)j / passCount;

      Float3 left, right;
      calculateSingleEdgePair(p1, p2, p3, p4, t, left, right);

      leftList.push_back(left);
      rightList.push_back(right);
   }
}

Model *createTrackSegment(std::vector<TrackPoint> &pointList, bool wrap) {
   int pCount = pointList.size();

   if (pCount <= 1) {
      return nullptr;
   }

   std::vector<Float3> leftList, rightList;

   if (pCount >= 3) {
      computeEdges(
         pointList[pCount - 1], pointList[0],
         pointList[1], pointList[2],
         leftList, rightList);
   }
   else {
      computeEdges(
         pointList[0], pointList[0], 
         pointList[1], pointList[1], 
         leftList, rightList);//only 2 vertices
   }
   
   //generate position lsits of left and right edges
   for (int i = 1; i < pCount - 2; ++i) {
      computeEdges(
         pointList[i-1], pointList[i],
         pointList[i+1], pointList[1+2],
         leftList, rightList);
   }

   if (pCount >= 3) {
      computeEdges(
         pointList[pCount - 3], pointList[pCount - 2],
         pointList[pCount - 1], pointList[0],
         leftList, rightList);

      computeEdges(
         pointList[pCount - 2], pointList[pCount - 1],
         pointList[0], pointList[1],
         leftList, rightList);


   }

   //generate vbo and ibo
   std::vector<Float3> vertices;
   std::vector<int> indices;

   vertices.insert(vertices.end(), { leftList[0], rightList[0] });
   for (int i = 1; i < leftList.size(); ++i) {
      vertices.insert(vertices.end(), { leftList[i], rightList[i] });

      int v2 = i * 2;  //top right
      int v1 = v2 - 2; //top left  
      int v3 = v2 - 1; //bottom left
      int v4 = v2 + 1; //bottom right

      indices.insert(indices.end(), {v1, v2, v3, v2, v4, v3});
   }

   //calculate normals and push to vbo
   std::vector<FVF_Pos3_Norm3_Col4> outV;
   std::vector<Float3> normals(vertices.size());
   std::vector<int> normalCounts(vertices.size());

   calculateNormals(vertices, indices, normals, normalCounts);

   for (int i = 0; i < vertices.size(); ++i) {
      Float3 normal;

      if (normalCounts[i] > 0) {
         normal = vec::mul(normals[i], 1.0f / normalCounts[i]);
      }

      outV.push_back({ vertices[i], normal, CommonColors::White });
   }

   return ModelManager::create(
      outV.data(),
      outV.size(),
      indices.data(), 
      indices.size());
}