#include "Model.hpp"

#include <vector>
#include <list>

struct FaCE;

struct AdjFace {
   int verts[3];
   Plane pln;
   //static AdjFace fromFace(Face const &f);
};

struct Face {
   int verts[3];
   Plane pln;
   std::vector<int> points;
   
   //0->1, 1->2, 2->0
   AdjFace adjacency[3];

   void buildPlane(std::vector<Float3> &pointCloud) {
      pln = Plane::fromFace(pointCloud[verts[0]], pointCloud[verts[1]], pointCloud[verts[2]]);
   }
};

//AdjFace AdjFace::fromFace(Face const &f) {
//   return{ {f.verts[0], f.verts[1], f.verts[2]}, f.pln };
//}

typedef std::vector<Float3> PointCloud;
typedef std::list<Face> FaceList;

struct QuickHull {
   PointCloud &points;
   FaceList faces, emptyFaces;
};

static bool faceAdjOnEdge(Face const &f1, Face const &f2, int v1, int v2) {
   int matches = 0;
   for (int i = 0; i < 3; ++i) {
      if (f1.verts[v1] == f2.verts[i] && ++matches == 2) { return true; }
      if (f1.verts[v2] == f2.verts[i] && ++matches == 2) { return true; }
   }
   return false;
}

static bool faceAdj(Face const &f1, Face const &f2) {
   int matches = 0;
   for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
         if (f1.verts[i] == f2.verts[j] && ++matches == 2) { return true; }
      }
   }
   return false;
}

//adds the first 4 faces
static void qhInit(QuickHull &qh) {

   //extreme indices
   int EP[6] = { -1,-1,-1,-1,-1,-1 };
   enum { minX, minY, minZ, maxX, maxY, maxZ };

   int i = 0;
   for (auto &p : qh.points) {
      if (EP[minX] < 0 || p.x < qh.points[EP[minX]].x) { EP[minX] = i; }
      if (EP[maxX] < 0 || p.x > qh.points[EP[maxX]].x) { EP[maxX] = i; }
      if (EP[minY] < 0 || p.y < qh.points[EP[minY]].y) { EP[minY] = i; }
      if (EP[maxY] < 0 || p.y > qh.points[EP[maxY]].y) { EP[maxY] = i; }
      if (EP[minZ] < 0 || p.z < qh.points[EP[minZ]].z) { EP[minZ] = i; }
      if (EP[maxZ] < 0 || p.z > qh.points[EP[maxZ]].z) { EP[maxZ] = i; }
      ++i;
   }

   //find two EP's with maximum dist for base line
   float dist = 0.0f;
   Face base;
   for (int i = 0; i < 6; ++i) {
      Float3 &p1 = qh.points[EP[i]];
      for (int j = 0; j < 6; ++j) {
         if (i == j) { continue; }

         Float3 &p2 = qh.points[EP[j]];
         float d = vec::len(vec::sub(p1, p2));
         if (d > dist) {
            dist = d;
            base.verts[0] = EP[i];
            base.verts[1] = EP[j];
         }
      }
   }

   //find EP furthest from base line for 1st face
   dist = 0.0f;
   Float3 &v1 = qh.points[base.verts[0]];
   Float3 &v2 = qh.points[base.verts[1]];

   for (int i = 0; i < 6; ++i) {
      if (EP[i] == base.verts[0] || EP[i] == base.verts[1]) {
         continue;
      }

      float d = fabs(vec::distPoint2LineSegment(qh.points[EP[i]], v1, v2));
      if (d > dist) {
         dist = d;
         base.verts[2] = EP[i];
      }
   }

   Float3 &v3 = qh.points[base.verts[2]];

   //find point in cloud furthest from first face
   i = 0;
   dist = 0.0f;
   auto baseNormal = vec::faceNormal(v1, v2, v3);
   int fourth = -1;
   for (auto &p : qh.points) {
      float d = fabs(vec::distPoint2FaceWithNormal(p, v1, v2, v3, baseNormal));
      if (d > dist) {
         dist = d;
         fourth = i;
      }
      ++i;
   }

   Face f1 = { base.verts[0], base.verts[2], base.verts[1] };
   Face f2 = { fourth, base.verts[2], base.verts[0] };
   Face f3 = { fourth, base.verts[1], base.verts[2] };
   Face f4 = { fourth, base.verts[0], base.verts[1] };

   f1.buildPlane(qh.points);
   f2.buildPlane(qh.points);
   f3.buildPlane(qh.points);
   f4.buildPlane(qh.points);

   //f1.adjacency[0] = AdjFace::fromFace(f2);
   //f2.adjacency[1] = AdjFace::fromFace(f1);

   //f1.adjacency[1] = AdjFace::fromFace(f3);
   //f3.adjacency[1] = AdjFace::fromFace(f1);

   //f1.adjacency[2] = AdjFace::fromFace(f4);
   //f4.adjacency[1] = AdjFace::fromFace(f1);

   //f2.adjacency[0] = AdjFace::fromFace(f3);
   //f3.adjacency[2] = AdjFace::fromFace(f2);

   //f2.adjacency[2] = AdjFace::fromFace(f4);
   //f4.adjacency[0] = AdjFace::fromFace(f2);

   //f3.adjacency[0] = AdjFace::fromFace(f4);
   //f4.adjacency[2] = AdjFace::fromFace(f3);

   //push first 4 faces
   FaceList tFaces;
   tFaces.insert(tFaces.end(), {f1, f2, f3, f4});

   //split all points into face pointlists
   i = 0;
   for (auto &p : qh.points) {
      int fi = 0;
      for (auto &f : tFaces) {
         if (!Plane::behind(f.pln, p)) {
            f.points.push_back(i);
            break;
         }
      }
      ++i;
   }

   //move non-empty faces to the face stack
   for (auto &f : tFaces) {
      if (!f.points.empty()) {
         qh.faces.push_back(std::move(f));
      }
      else {
         qh.emptyFaces.push_back(std::move(f));
      }
   }
}

void qhIteration(QuickHull &qh) {
   Face currentFace = std::move(qh.faces.front());
   qh.faces.pop_front();

   //shouldnt happen but, push out an empty face and return
   if (currentFace.points.empty()) {
      qh.emptyFaces.push_back(std::move(currentFace));
      return;
   }

   auto &v1 = qh.points[currentFace.verts[0]];
   auto &v2 = qh.points[currentFace.verts[1]];
   auto &v3 = qh.points[currentFace.verts[2]];

   //find the furthest point index
   float dist = 0.0f;
   int furthestIndex = -1;


   for (auto &pi : currentFace.points) {
      auto &p = qh.points[pi];

      float d = fabs(vec::distPoint2FaceWithNormal(p, v1, v2, v3, currentFace.pln.normal));
      if (d >= dist) {
         dist = d;
         furthestIndex = pi;
      }
   }

   //find all faces the point is in front of
   auto &furthest = qh.points[furthestIndex];
   std::vector<Face> visibleFaces;

   

   //for (auto f = qh.faces.begin(); f != qh.faces.end();) {
   //   if (!Plane::behind(f->pln, furthest)) {
   //      visibleFaces.push_back(*f);

   //      if (Plane::behind(f->adjacency[0].pln, furthest)) {
   //         Face newFace = {};
   //      }

   //      f = qh.faces.erase(f);
   //   }
   //   else {
   //      ++f;
   //   }
   //}







   for (auto f = qh.faces.begin(); f != qh.faces.end();) {
      if (faceAdj(*f, currentFace) && !Plane::behind(f->pln, furthest)) {
         visibleFaces.push_back(*f);
         f = qh.faces.erase(f);
      }
      else {
         ++f;
      }
   }


   //create the new faces by remo9viung the closest edge
   FaceList newFaces;

   if (visibleFaces.empty() && !qh.faces.empty()) {
      //no visible faces,  pyramid off the base
      visibleFaces.push_back(currentFace);
      newFaces.push_back({ currentFace.verts[0], furthestIndex, currentFace.verts[2] });
      newFaces.push_back({ currentFace.verts[1], furthestIndex, currentFace.verts[0] });
      newFaces.push_back({ currentFace.verts[2], furthestIndex, currentFace.verts[1] });
   }
   else {
      visibleFaces.push_back(currentFace);
      for (auto &f : visibleFaces) {
         auto &v1 = qh.points[f.verts[0]];
         auto &v2 = qh.points[f.verts[1]];
         auto &v3 = qh.points[f.verts[2]];
         float d1 = vec::distPoint2LineSegment(furthest, v1, v2);
         float d2 = vec::distPoint2LineSegment(furthest, v1, v3);
         float d3 = vec::distPoint2LineSegment(furthest, v2, v3);

         if (d1 < d2 && d1 < d3) {
            newFaces.push_back({ f.verts[0], furthestIndex, f.verts[2] });
            newFaces.push_back({ f.verts[2], furthestIndex, f.verts[1] });
         }
         else if (d2 < d1 && d2 < d3) {
            newFaces.push_back({ f.verts[1], furthestIndex, f.verts[0] });
            newFaces.push_back({ f.verts[2], furthestIndex, f.verts[1] });
         }
         else if (d3 < d1 && d3 < d2) {
            newFaces.push_back({ f.verts[1], furthestIndex, f.verts[0] });
            newFaces.push_back({ f.verts[0], furthestIndex, f.verts[2] });
         }
      }
   }
   

   //calculate normals
   for (auto &nf : newFaces) {
      nf.buildPlane(qh.points);
   }

   //dispense points
   for (auto &f : visibleFaces) {
      for (auto &pi : f.points) {
         for (auto &nf : newFaces) {
            if (!Plane::behind(nf.pln, qh.points[pi])) {
               nf.points.push_back(pi);
               break;
            }
         }
      }
   }

   //move non-empty faces to the face stack
   for (auto &f : newFaces) {
      if (!f.points.empty()) {
         qh.faces.push_back(std::move(f));
      }
      else {
         qh.emptyFaces.push_back(std::move(f));
      }
   }
}

QuickHullTestModels quickHullTest(PointCloud &points, int iterCount) {

   QuickHull qh = { points };

   qhInit(qh);

   while (iterCount-- && !qh.faces.empty()) {
      qhIteration(qh);
   }

   //while (!qh.faces.empty()) { qhIteration(qh); }
   

   QuickHullTestModels out;

   std::vector<FVF_Pos3_Col4> faceLines;
   std::vector<FVF_Pos3_Col4> pointLines;

   ColorRGBAf colors[8] = { CommonColors::Red , CommonColors::DkRed , CommonColors::DkGreen, 
      CommonColors::Blue , CommonColors::DkBlue , CommonColors::Cyan , CommonColors::Yellow , CommonColors::Magenta };


   int fi = 0;
   for (auto &f : qh.faces) {

      ColorRGBAf color = colors[rand()%8];

      faceLines.insert(faceLines.end(), {
         { qh.points[f.verts[0]], CommonColors::White },
         { qh.points[f.verts[1]], CommonColors::White },
         { qh.points[f.verts[1]], CommonColors::White },
         { qh.points[f.verts[2]], CommonColors::White },
         { qh.points[f.verts[2]], CommonColors::White },
         { qh.points[f.verts[0]], CommonColors::White }
      });

      auto c = vec::centroid(points[f.verts[0]], points[f.verts[1]], points[f.verts[2]]);
      faceLines.insert(faceLines.end(), {
         { c, color },
         { vec::add(c, vec::mul(f.pln.normal, 0.05f)), color }
      });

      for (auto &p : f.points) {
         pointLines.push_back({ points[p],  color });
      }

      ++fi;
   }


   fi = 0;
   for (auto &f : qh.emptyFaces) {

      faceLines.insert(faceLines.end(), {
         { qh.points[f.verts[0]], CommonColors::Green },
         { qh.points[f.verts[1]], CommonColors::Green },
         { qh.points[f.verts[1]], CommonColors::Green },
         { qh.points[f.verts[2]], CommonColors::Green },
         { qh.points[f.verts[2]], CommonColors::Green },
         { qh.points[f.verts[0]], CommonColors::Green }
      });

      //auto c = vec::centroid(points[f.vertices[0]], points[f.vertices[1]], points[f.vertices[2]]);
      //faceLines.insert(faceLines.end(), {
      //   { c, CommonColors::Red },
      //   { vec::add(c, vec::mul(f.pln.normal, 0.05f)), CommonColors::Red }
      //});

      ++fi;
   }

   out.lineModels.push_back(ModelManager::create(faceLines));
   out.pointModels.push_back(ModelManager::create(pointLines));

   return out;
}