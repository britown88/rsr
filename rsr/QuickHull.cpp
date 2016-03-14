#include "Model.hpp"

#include <vector>

struct Face {
   int vertices[3];
   Plane p;
   std::vector<int> points;
};

std::vector<Float3> quickHull(std::vector<Float3> &pointCloud) {

   //extreme indices
   int EP[6] = { -1,-1,-1,-1,-1,-1 };
   enum {minX, minY, minZ, maxX, maxY, maxZ};

   int i = 0;
   for (auto &p : pointCloud) {
      if (EP[minX] < 0 || p.x < pointCloud[EP[minX]].x) { EP[minX] = i; }
      if (EP[maxX] < 0 || p.x > pointCloud[EP[maxX]].x) { EP[maxX] = i; }
      if (EP[minY] < 0 || p.y < pointCloud[EP[minY]].y) { EP[minY] = i; }
      if (EP[maxY] < 0 || p.y > pointCloud[EP[maxY]].y) { EP[maxY] = i; }
      if (EP[minZ] < 0 || p.z < pointCloud[EP[minZ]].z) { EP[minZ] = i; }
      if (EP[maxZ] < 0 || p.z > pointCloud[EP[maxZ]].z) { EP[maxZ] = i; }
      ++i;
   }

   float dist = 0.0f;
   Face base;
   for (int i = 0; i < 6; ++i) {
      Float3 &p1 = pointCloud[EP[i]];
      for (int j = 0; j < 6; ++j) {
         if (i == j) { continue; }

         Float3 &p2 = pointCloud[EP[j]];
         float d = vec::len(vec::sub(p1, p2));
         if (d > dist) {
            dist = d;
            base.vertices[0] = EP[i];
            base.vertices[1] = EP[j];
         }
      }
   }

   dist = 0.0f;
   Float3 &v1 = pointCloud[base.vertices[0]];
   Float3 &v2 = pointCloud[base.vertices[1]];
   auto v1v2 = vec::sub(v2, v1);
   float v1v2Len = vec::len(v1v2);

   for (int i = 0; i < 6; ++i) {
      if (EP[i] == base.vertices[0] || EP[i] == base.vertices[1]) {
         continue;
      }

      float d = fabs(vec::distPoint2LineSegment(pointCloud[EP[i]], v1, v2));
      if (d > dist) {
         dist = d;
         base.vertices[2] = EP[i];
      }
   }

   Float3 &v3 = pointCloud[base.vertices[2]];

   std::vector<Face> faces;
   faces.push_back(base);

   i = 0;
   dist = 0.0f;
   Plane basePlane = { v1, vec::faceNormal(v1, v2, v3) };
   auto baseNormal = vec::faceNormal(v1, v2, v3);
   int fourth = -1;
   for (auto &p : pointCloud) {
      float d = fabs(vec::distPoint2FaceWithNormal(p, v1, v2, v3, baseNormal));
      if (d > dist) {
         dist = d;
         fourth = i;
      }
      ++i;
   }

   faces.insert(faces.end(), { 
      { base.vertices[0], fourth, base.vertices[1] },
      { base.vertices[0], fourth, base.vertices[2] },
      { base.vertices[1], fourth, base.vertices[2] } });

   std::vector<Float3> out;
   for (auto &f : faces) {
      out.insert(out.end(), {
         pointCloud[f.vertices[0]],
         pointCloud[f.vertices[1]],
         pointCloud[f.vertices[1]],
         pointCloud[f.vertices[2]],
         pointCloud[f.vertices[2]],
         pointCloud[f.vertices[0]]
      });
   }

   return out;
}