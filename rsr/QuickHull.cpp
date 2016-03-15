#include "Model.hpp"

#include <vector>
#include <list>

typedef std::vector<Float3> PointCloud;
typedef std::list<int> FaceList;

struct Face {
   int verts[3];
   Plane pln;
   std::vector<int> points;
   
   //edges
   enum {
      Edge0_1,
      Edge1_2,
      Edge2_0
   };

   //indices into qh face vector (3 edges)
   int adjFaces[3];
   

   void buildPlane(std::vector<Float3> &pts) {
      pln = Plane::fromFace(pts[verts[0]], pts[verts[1]], pts[verts[2]]);
   }

   //given a faceindex f and one of its edges, return the index into
   //the adjacent face's edge list that connects to f
   static int oppositeEdgeIndex(int f, int edge, std::vector<Face> const &faces) {
      auto &f1 = faces[f];
      auto &f2 = faces[f1.adjFaces[edge]];

      return f2.adjFaces[0] == f ? 0 : (f2.adjFaces[1] == f ? 1 : 2);
   }
};

struct QuickHull {
   PointCloud &points;
   std::vector<Face> faces;
   FaceList open, closed;
};

//static bool faceAdjOnEdge(Face const &f1, Face const &f2, int v1, int v2) {
//   int matches = 0;
//   for (int i = 0; i < 3; ++i) {
//      if (f1.verts[v1] == f2.verts[i] && ++matches == 2) { return true; }
//      if (f1.verts[v2] == f2.verts[i] && ++matches == 2) { return true; }
//   }
//   return false;
//}
//
//static bool faceAdj(Face const &f1, Face const &f2) {
//   int matches = 0;
//   for (int i = 0; i < 3; ++i) {
//      for (int j = 0; j < 3; ++j) {
//         if (f1.verts[i] == f2.verts[j] && ++matches == 2) { return true; }
//      }
//   }
//   return false;
//}

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

   qh.faces.insert(qh.faces.end(), {

        //0            //1            //2
      { base.verts[0], base.verts[2], base.verts[1] },//0
      { fourth,        base.verts[2], base.verts[0] },//1
      { fourth,        base.verts[1], base.verts[2] },//2
      { fourth,        base.verts[0], base.verts[1] } });//3

   for (auto &f : qh.faces) {
      f.buildPlane(qh.points);
   }

   //set starting adjacency manually (setting to indices of other faces)
   qh.faces[0].adjFaces[Face::Edge0_1] = 1;
   qh.faces[0].adjFaces[Face::Edge1_2] = 2;
   qh.faces[0].adjFaces[Face::Edge2_0] = 3;

   qh.faces[1].adjFaces[Face::Edge0_1] = 2;
   qh.faces[1].adjFaces[Face::Edge1_2] = 0;
   qh.faces[1].adjFaces[Face::Edge2_0] = 3;

   qh.faces[2].adjFaces[Face::Edge0_1] = 3;
   qh.faces[2].adjFaces[Face::Edge1_2] = 0;
   qh.faces[2].adjFaces[Face::Edge2_0] = 1;

   qh.faces[3].adjFaces[Face::Edge0_1] = 1;
   qh.faces[3].adjFaces[Face::Edge1_2] = 0;
   qh.faces[3].adjFaces[Face::Edge2_0] = 2;

   //push first 4 faces
   FaceList tFaces;
   tFaces.insert(tFaces.end(), {0, 1, 2, 3});

   //split all points into face pointlists
   i = 0;
   for (auto &p : qh.points) {
      int fi = 0;
      for (auto &fi : tFaces) {
         auto &f = qh.faces[fi];
         if (!Plane::behind(f.pln, p)) {
            f.points.push_back(i);
            break;
         }
      }
      ++i;
   }

   //move non-empty faces to the face stack
   for (auto &fi : tFaces) {
      auto &f = qh.faces[fi];
      if (!f.points.empty()) {
         qh.open.push_back(fi);
      }
      else {
         qh.closed.push_back(fi);
      }
   }
}

void qhIteration(QuickHull &qh) {
   int cfIndex = qh.open.front();
   Face &currentFace = qh.faces[cfIndex];
   qh.open.pop_front();

   //shouldnt happen but, push out an empty face and return
   if (currentFace.points.empty()) {
      qh.closed.push_back(cfIndex);
      return;
   }

   //find the furthest point index from the current face
   float dist = 0.0f;
   int furthestIndex = -1;

   for (auto &pi : currentFace.points) {
      auto &p = qh.points[pi];

      float d = fabs(vec::distPoint2FaceWithNormal(p, 
         qh.points[currentFace.verts[0]],
         qh.points[currentFace.verts[1]],
         qh.points[currentFace.verts[2]],
         currentFace.pln.normal));

      if (d >= dist) {
         dist = d;
         furthestIndex = pi;
      }
   }
   
   auto &furthest = qh.points[furthestIndex];

   std::vector<bool> visited(qh.faces.size(), false);
   FaceList lightFaces, discardedFaces, newFaces;

   struct HorizonEdge {
      int litFace, unlitFace, litEdge;
   };

   std::vector<HorizonEdge> horizon;

   //now we traverse each adjacent face starting with th current
   //if the adjacent face is dark we add a horizon edge
   lightFaces.push_back(cfIndex);

   while (!lightFaces.empty()) {
      int lfi = lightFaces.front();
      lightFaces.pop_front();

      auto &lf = qh.faces[lfi];

      for (int edge = 0; edge < 3; ++edge) {
         int adjFacei = lf.adjFaces[edge];
         if (!visited[adjFacei]) {
            auto &adjFace = qh.faces[adjFacei];

            if (adjFace.points.empty()) {
               return;
            }

            if (Plane::behind(adjFace.pln, furthest)) {
               //horizon edge found
               horizon.push_back({lfi, adjFacei, edge});
            }
            else {
               //new lightface
               lightFaces.push_back(adjFacei);
            }
         }
      }

      discardedFaces.push_back(lfi);
      visited[lfi] = true;
   }
   
   //create the new faces along the horizon
   for (auto &edge : horizon) {
      auto &lit = qh.faces[edge.litFace];
      auto &unlit = qh.faces[edge.unlitFace];

      Face newface;

      switch (edge.litEdge) {         
      case Face::Edge0_1:
         newface.verts[0] = lit.verts[1];
         newface.verts[1] = furthestIndex;
         newface.verts[2] = lit.verts[0];
         break;

      case Face::Edge1_2:
         newface.verts[0] = lit.verts[2];
         newface.verts[1] = furthestIndex;
         newface.verts[2] = lit.verts[1];
         break;

      case Face::Edge2_0:
         newface.verts[0] = lit.verts[0];
         newface.verts[1] = furthestIndex;
         newface.verts[2] = lit.verts[2];
         break;
      }

      newface.buildPlane(qh.points);

      int newIndex = qh.faces.size();
      newface.adjFaces[Face::Edge2_0] = edge.unlitFace;
      unlit.adjFaces[Face::oppositeEdgeIndex(edge.litFace, edge.litEdge, qh.faces)] = newIndex;
      newFaces.push_back(newIndex);  
      
      qh.faces.push_back(std::move(newface));
   }

   //link the new faces adjacency
   int faceCount = qh.faces.size();
   int start = faceCount - newFaces.size();
   for (int i = start; i < faceCount; ++i) {
      qh.faces[i].adjFaces[Face::Edge0_1] = i + 1 < faceCount ? i + 1 : start;
      qh.faces[i].adjFaces[Face::Edge1_2] = i > start ? i - 1 : faceCount - 1;
   }

   for (auto &i : discardedFaces) {
      for (auto &pi : qh.faces[i].points) {
         for (auto &nfi : newFaces) {
            auto &nf = qh.faces[nfi];
            if (!Plane::behind(nf.pln, qh.points[pi])) {
               nf.points.push_back(pi);
            }
         }
      }

      qh.faces[i].points.clear();
   }

   for (auto &f : newFaces) {
      if (!qh.faces[f].points.empty()) {
         qh.open.push_back(f);
      }
      else {
         qh.closed.push_back(f);
      }
   }

   for (auto iter = qh.open.begin(); iter != qh.open.end();) {
      if (qh.faces[*iter].points.empty()) {
         iter = qh.open.erase(iter);
      }
      else {
         ++iter;
      }
   }

}

QuickHullTestModels quickHullTest(PointCloud &points, int iterCount) {

   QuickHull qh = { points };

   qhInit(qh);

   while (iterCount-- && !qh.open.empty()) {
      qhIteration(qh);
   }

   //while (!qh.faces.empty()) { qhIteration(qh); }
   

   QuickHullTestModels out;

   std::vector<FVF_Pos3_Col4> faceLines;
   std::vector<FVF_Pos3_Col4> pointLines;
   ModelVertices polys;

   ColorRGBAf colors[5] = { CommonColors::Red, 
      CommonColors::Blue , CommonColors::Cyan , CommonColors::Yellow , CommonColors::Magenta };


   int fi = 0;
   for (auto &fi : qh.open) {
      auto &f = qh.faces[fi];

      ColorRGBAf color = colors[rand()%5];

      faceLines.insert(faceLines.end(), {
         { qh.points[f.verts[0]], CommonColors::White },
         { qh.points[f.verts[1]], CommonColors::White },
         { qh.points[f.verts[1]], CommonColors::White },
         { qh.points[f.verts[2]], CommonColors::White },
         { qh.points[f.verts[2]], CommonColors::White },
         { qh.points[f.verts[0]], CommonColors::White }
      });

      int vCount = polys.positions.size();

      polys.positions.insert(polys.positions.end(), {
         qh.points[f.verts[0]],
         qh.points[f.verts[1]],
         qh.points[f.verts[2]]});

      polys.positionIndices.insert(polys.positionIndices.end(), {
         vCount + 0, vCount + 1,vCount + 2
      });


      auto c = vec::centroid(points[f.verts[0]], points[f.verts[1]], points[f.verts[2]]);
      faceLines.insert(faceLines.end(), {
         { c, color },
         { vec::add(c, vec::mul(f.pln.normal, 0.02f)), color }
      });

      for (auto &p : f.points) {
         pointLines.push_back({ points[p],  color });
      }

      ++fi;
   }


   fi = 0;
   for (auto &fi : qh.closed) {
      auto &f = qh.faces[fi];

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


   auto poly = polys.calculateNormals().expandIndices().createModel(ModelOpts::IncludeColor | ModelOpts::IncludeNormals);
   out.polyModels.push_back(poly);

   return out;
}