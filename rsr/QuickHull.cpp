#include "Model.hpp"

#include <vector>
#include <list>
#include <algorithm>

typedef std::vector<Float3> PointCloud;
typedef std::list<int> FaceList;

struct Face {
   std::vector<int> verts;
   std::vector<int> adjFaces;

   Plane pln;
   std::vector<int> points;

   void buildPlane(std::vector<Float3> &pts) {
      Float3 centroid, normal;
      size_t vCount = verts.size();

      centroid.x = pts[verts[0]].x + pts[verts[1]].x;
      centroid.y = pts[verts[0]].y + pts[verts[1]].y;
      centroid.z = pts[verts[0]].z + pts[verts[1]].z;

      normal = { 0.0f, 0.0f, 0.0f };

      for (int i = 0; i < verts.size() - 2; ++i) {
         auto &v1 = pts[verts[i]];
         auto &v2 = pts[verts[i + 1]];
         auto &v3 = pts[verts[i + 2]];

         centroid = vec::add(centroid, v3);
         normal = vec::add(normal, vec::faceNormal(v1, v2, v3));
      }

      pln.normal = vec::normal(vec::mul(normal, 1.0f / (vCount - 2)));
      pln.orig = pts[verts[0]];
   }

   static int clampedEdge(Face const &f, int edge) {
      int eCount = f.adjFaces.size();
      while (edge < 0) {
         edge += eCount;
      }
      return edge % eCount;
   }

   //given a faceindex f and one of its edges, return the index into
   //the adjacent face's edge list that connects to f
   static int oppositeEdgeIndex(int f, int edge, std::vector<Face> const &faces) {
      auto &f1 = faces[f];
      auto &f2 = faces[f1.adjFaces[edge]];

      int e = 0;
      for (auto face : f2.adjFaces) {
         if (face == f) {
            if (f2.verts[e] == f1.verts[clampedEdge(f1, edge + 1)] &&
               f2.verts[clampedEdge(f2, e + 1)] == f1.verts[edge]) {
               return e;
            }
         }
         ++e;
      }

      return -1;
   }

   

   static void getEdgeVertices(Face const &f, int edge, int &v1, int &v2) {

      v1 = f.verts[edge];
      v2 = f.verts[clampedEdge(f, edge + 1)];
   }

   static bool coPlanar(Face const &f1, Face const &f2) {
      return vec::dot(f1.pln.normal, f2.pln.normal) > 0.99999f;
   }

   static int merge(int f1i, int f2i, int edge, std::vector<Face> &faces) {

      int newIndex = faces.size();
      faces.push_back({});
      Face &newFace = faces[newIndex];

      auto &f1 = faces[f1i];
      auto &f2 = faces[f2i];
      int oppositeEdge = oppositeEdgeIndex(f1i, edge, faces);

      int edgeLen = 0;
      for (int i = 0; i < f1.verts.size(); ++i) {
         int nextEdge = clampedEdge(f1, edge + i);
         if(f1.adjFaces[nextEdge] == f2i) {
            ++edgeLen;
         }
         else {
            break;
         }
      }

      int v1 = f1.verts[edge];
      int v2 = f1.verts[clampedEdge(f1, edge + edgeLen)];

      newFace.verts.push_back(v1);
      int current = clampedEdge(f2, oppositeEdge + 2);

      while(current != clampedEdge(f2, oppositeEdge + 2 - edgeLen)) {
         
         newFace.verts.push_back(f2.verts[current]);

         int lastEdge = clampedEdge(f2, current - 1);
         int adj = f2.adjFaces[lastEdge];
         newFace.adjFaces.push_back(adj);
         faces[adj].adjFaces[oppositeEdgeIndex(f2i, lastEdge, faces)] = newIndex;

         current = clampedEdge(f2, current + 1);
      }

      current = clampedEdge(f1, edge + 1 + edgeLen);

      while (current != clampedEdge(f1, edge + 1)) {
         int lastEdge = clampedEdge(f1, current - 1);

         if (current != edge) {
            newFace.verts.push_back(f1.verts[current]);
         }
         
         int adj = f1.adjFaces[lastEdge];
         newFace.adjFaces.push_back(adj);
         faces[adj].adjFaces[oppositeEdgeIndex(f1i, lastEdge, faces)] = newIndex;

         current = clampedEdge(f1, current + 1);
      }
           
      return newIndex;
   }
};

struct QuickHull {
   PointCloud &points;
   std::vector<Face> faces;
   FaceList open, closed;

   PointCloud horizon;
   Float3 summit;
   Face currentWorking;
   FaceList coplanars;
};

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
   base.verts.resize(3);
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
      { {base.verts[0], base.verts[2], base.verts[1]} },//0
      { {fourth,        base.verts[2], base.verts[0]} },//1
      { {fourth,        base.verts[1], base.verts[2]} },//2
      { {fourth,        base.verts[0], base.verts[1]} } });//3

   for (auto &f : qh.faces) {
      f.buildPlane(qh.points);
   }

   //set starting adjacency manually (setting to indices of other faces)
   qh.faces[0].adjFaces.insert(qh.faces[0].adjFaces.end(), { 1, 2, 3 });
   qh.faces[1].adjFaces.insert(qh.faces[1].adjFaces.end(), { 2, 0, 3 });
   qh.faces[2].adjFaces.insert(qh.faces[2].adjFaces.end(), { 3, 0, 1 });
   qh.faces[3].adjFaces.insert(qh.faces[3].adjFaces.end(), { 1, 0, 2 });

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
   qh.currentWorking = currentFace;
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

   if (dist <= 0.0001f) {
      //only point is within epsilon, we're done here
      qh.closed.push_back(cfIndex);
      return;
   }
   
   auto &furthest = qh.points[furthestIndex];

   std::vector<bool> visited(qh.faces.size(), false);
   FaceList lightFaces, discardedFaces, newFaces;

   struct HorizonEdge {
      int litFace, litEdge;
   };

   std::vector<HorizonEdge> horizon;

   //now we traverse each adjacent face starting with th current
   //if the adjacent face is dark we add a horizon edge

   struct OpenFace { int index, edge, edgeCount; int fromIndex, fromEdge; };
   std::list<OpenFace> openFaces;

   openFaces.push_back({ cfIndex, 0, 0 });
   while (!openFaces.empty()) {
      auto &of = openFaces.back();
      auto &ofFace = qh.faces[of.index];
      int ofEdgeCount = ofFace.adjFaces.size();
      

      if (Plane::behind(qh.faces[of.index].pln, furthest)) {
         //point not visible, make our edge
         horizon.push_back({of.fromIndex, of.fromEdge});
         openFaces.pop_back();
      }
      else if (of.edgeCount == ofEdgeCount){
         discardedFaces.push_back(of.index);
         openFaces.pop_back();
      }
      else {
         visited[of.index] = true;

         while (of.edgeCount < ofEdgeCount) {
            int adj = qh.faces[of.index].adjFaces[of.edge];

            if (visited[adj]) {//skip visited
               of.edge = Face::clampedEdge(ofFace, of.edge + 1);
               ++of.edgeCount;
               continue;
            }

            int opp = Face::oppositeEdgeIndex(of.index, of.edge, qh.faces);

            if (opp == -1) {
               int i = 0;
               int j = *(int*)i;//picnic
            }
            
            openFaces.push_back({ adj, opp, 0, of.index, of.edge });
            of.edge = Face::clampedEdge(ofFace, of.edge + 1);
            ++of.edgeCount;
            break;
         }
      }

      
   }

   qh.horizon.clear();
   qh.summit = furthest;
   
   //create the new faces along the horizon
   for (auto &edge : horizon) {
      auto &lit = qh.faces[edge.litFace];
      int unliti = lit.adjFaces[edge.litEdge];
      auto &unlit = qh.faces[unliti];

      Face newface;

      int edgeVertices[2] = { 0 };
      Face::getEdgeVertices(lit, edge.litEdge, edgeVertices[0], edgeVertices[1]);

      newface.verts.insert(newface.verts.end(), { edgeVertices[0], edgeVertices[1], furthestIndex });
      newface.adjFaces.resize(3, -1);

      qh.horizon.push_back(qh.points[newface.verts[0]]);
      qh.horizon.push_back(qh.points[newface.verts[1]]);

      newface.buildPlane(qh.points);

      int newIndex = qh.faces.size();
      newface.adjFaces[0] = unliti;
      int opposite = Face::oppositeEdgeIndex(edge.litFace, edge.litEdge, qh.faces);
      unlit.adjFaces[opposite] = newIndex;
      newFaces.push_back(newIndex); 

      qh.faces.push_back(newface);
   }

   //link the new faces adjacency as triangles
   int faceCount = qh.faces.size();
   int start = faceCount - newFaces.size();
   for (int i = start; i < faceCount; ++i) {
      int i2 = (i + 1 < faceCount) ? i + 1 : start;
      auto &f1 = qh.faces[i];
      auto &f2 = qh.faces[i2];

      f1.adjFaces[1] = i2;
      f2.adjFaces[2] = i;
   }

   //go through the triangles and merge coplanars
   for (auto nf = newFaces.begin(); nf != newFaces.end();) {
      auto &f1 = qh.faces[*nf];
      int edge = 0;

      bool removed = false;
      for (auto adj : f1.adjFaces) {
         auto &f2 = qh.faces[adj];

         if (Face::coPlanar(f1, f2)) {

            auto merged = Face::merge(*nf, adj, edge, qh.faces);

            auto &mf = qh.faces[merged];
            mf.buildPlane(qh.points);

            for (int i = 0; i < mf.adjFaces.size(); ++i) {
               if (Face::oppositeEdgeIndex(merged, i, qh.faces) == -1) {
                  break;
               }
            }

            int adjIsNew = false;
            for (auto iter = newFaces.begin(); iter != newFaces.end(); ++iter) {
               if (*iter == adj) {
                  newFaces.erase(iter);
                  adjIsNew = true;
                  break;
               }
            }

            if (!adjIsNew) {
               discardedFaces.push_back(adj);
            }

            newFaces.push_back(merged);

            nf = newFaces.erase(nf);
            removed = true;
            break;
         }

         ++edge;
      }

      if (!removed) {
         ++nf;
      }
   }   


   for (auto &i : discardedFaces) {
      for (auto &pi : qh.faces[i].points) {
         for (auto &nfi : newFaces) {
            auto &nf = qh.faces[nfi];
            if (!Plane::behind(nf.pln, qh.points[pi])) {
               nf.points.push_back(pi);
               break;
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
   ModelVertices copolys;

   ColorRGBAf colors[5] = { CommonColors::Red, 
      CommonColors::Blue , CommonColors::Cyan , CommonColors::Yellow , CommonColors::Magenta };

   for (auto &fi : qh.coplanars) {
      auto &f = qh.faces[fi];
      int vCount = copolys.positions.size();

      ColorRGBAf lineColor = CommonColors::Black;
      ColorRGBAf faceColor = CommonColors::Red;

      float normScale = 0.05f;

      std::vector<Float3> points(f.verts.size());

      for (int i = 0; i < f.verts.size(); ++i) {
         points[i] = vec::add(qh.points[f.verts[i]], vec::mul(f.pln.normal, normScale));
      }

      for (int i = 0; i < f.verts.size(); ++i) {
         faceLines.push_back({ points[i], lineColor });
         faceLines.push_back({ points[Face::clampedEdge(f, i + 1)], lineColor });
      }

      for (int i = 0; i < f.verts.size() - 2; ++i) {
         copolys.colors.insert(copolys.colors.end(), { faceColor , faceColor , faceColor });
         copolys.positions.insert(copolys.positions.end(), {
            points[0],
            points[Face::clampedEdge(f, i + 1)],
            points[Face::clampedEdge(f, i + 2)]
         });

         copolys.positionIndices.insert(copolys.positionIndices.end(), {
            vCount + (i * 3) + 0, vCount + (i * 3) + 1,vCount + (i * 3) + 2
         });
      }
   }

   
   for (auto &fi : qh.open) {
      auto &f = qh.faces[fi];
      
      ColorRGBAf lineColor = CommonColors::DkGray;

      for (int i = 0; i < f.verts.size(); ++i) {
         faceLines.push_back({ qh.points[f.verts[i]], lineColor });
         faceLines.push_back({ qh.points[f.verts[Face::clampedEdge(f, i + 1)]], lineColor });
      }

      int vCount = polys.positions.size();

      int fCount = qh.faces.size();
      float colorFactor = 0.0f;
      if (fi > fCount - (qh.horizon.size() / 2)) {
         colorFactor = ((qh.horizon.size() / 2) - (fCount - fi)) / (float)(qh.horizon.size() / 2);
      }

      //ColorRGBAf faceColor = { 0.0f, 0.0f, colorFactor, 1.0f };
      ColorRGBAf faceColor = { 0.0f, 0.0f, 1.0f, 1.0f };


      for (int i = 0; i < f.verts.size() - 2; ++i) {
         polys.colors.insert(polys.colors.end(), { faceColor , faceColor , faceColor });
         polys.positions.insert(polys.positions.end(), {
            qh.points[f.verts[0]],
            qh.points[f.verts[Face::clampedEdge(f, i + 1)]],
            qh.points[f.verts[Face::clampedEdge(f, i + 2)]]
         });

         polys.positionIndices.insert(polys.positionIndices.end(), {
            vCount + ( i * 3 ) + 0, vCount + (i * 3) + 1,vCount + (i * 3) + 2
         });
      }

      ColorRGBAf color = CommonColors::Black;
      auto c = vec::centroid(points[f.verts[0]], points[f.verts[1]], points[f.verts[2]]);
      faceLines.insert(faceLines.end(), {
         { c, color },
         { vec::add(c, vec::mul(f.pln.normal, 0.02f)), color }
      });

      float factor = 0.0f;
      for (auto adji : f.adjFaces) {
         auto &adj = qh.faces[adji];
         auto c2 = vec::centroid(points[adj.verts[0]], points[adj.verts[1]], points[adj.verts[2]]);
         faceLines.insert(faceLines.end(), {
            { vec::add(c, vec::mul(f.pln.normal, 0.02f+ factor)), CommonColors::Red },
            { vec::add(c2, vec::mul(adj.pln.normal, 0.01f+ factor)), CommonColors::Yellow }
         });
         factor += 0.001f;
      }

      for (auto &p : f.points) {
         pointLines.push_back({ points[p],  CommonColors::White });
      }

   }

   if (!qh.horizon.empty()) {

      static float ptSize = 0.01f;
      
      faceLines.insert(faceLines.end(), {
         { { qh.summit.x, qh.summit.y - ptSize, qh.summit.z }, CommonColors::Green },
         { { qh.summit.x, qh.summit.y + ptSize, qh.summit.z }, CommonColors::Green },

         { { qh.summit.x, qh.summit.y, qh.summit.z - ptSize }, CommonColors::Green },
         { { qh.summit.x, qh.summit.y, qh.summit.z + ptSize }, CommonColors::Green },

         { { qh.summit.x - ptSize, qh.summit.y, qh.summit.z }, CommonColors::Green },
         { { qh.summit.x + ptSize, qh.summit.y, qh.summit.z }, CommonColors::Green },
      });

      //render horizon
      for(auto &p : qh.horizon){
         faceLines.push_back({ p, CommonColors::Green });
      }

      //render current working face

      std::vector<Float3> points(qh.currentWorking.verts.size());
      float normScale = 0.03f;
      for (int i = 0; i < qh.currentWorking.verts.size(); ++i) {
         points[i] = vec::add(qh.points[qh.currentWorking.verts[i]], vec::mul(qh.currentWorking.pln.normal, normScale));
      }

      ColorRGBAf lineColor = CommonColors::Magenta;
      for (int i = 0; i < qh.currentWorking.verts.size(); ++i) {
         faceLines.push_back({ points[i], lineColor });
         faceLines.push_back({ points[Face::clampedEdge(qh.currentWorking, i + 1)], lineColor });
      }
      
   }


   for (auto &fi : qh.closed) {
      auto &f = qh.faces[fi];

      ColorRGBAf color = CommonColors::Red;
      ColorRGBAf lineColor = CommonColors::Black;

      for (int i = 0; i < f.verts.size(); ++i) {
         faceLines.push_back({ qh.points[f.verts[i]], lineColor });
         faceLines.push_back({ qh.points[f.verts[Face::clampedEdge(f, i + 1)]], lineColor });
      }

      int vCount = polys.positions.size();

      float colorFactor = (fi / (float)qh.faces.size());
      //ColorRGBAf faceColor = { 0.0f, colorFactor, colorFactor, 1.0f };
      ColorRGBAf faceColor = CommonColors::Cyan;

      for (int i = 0; i < f.verts.size() - 2; ++i) {
         polys.colors.insert(polys.colors.end(), { faceColor , faceColor , faceColor });
         polys.positions.insert(polys.positions.end(), {
            qh.points[f.verts[0]],
            qh.points[f.verts[Face::clampedEdge(f, i + 1)]],
            qh.points[f.verts[Face::clampedEdge(f, i + 2)]]
         });

         polys.positionIndices.insert(polys.positionIndices.end(), {
            vCount + (i * 3) + 0, vCount + (i * 3) + 1,vCount + (i * 3) + 2
         });
      }

      //auto c = vec::centroid(points[f.verts[0]], points[f.verts[1]], points[f.verts[2]]);
      //faceLines.insert(faceLines.end(), {
      //   { c, color },
      //   { vec::add(c, vec::mul(f.pln.normal, 0.02f)), color }
      //});

      //for (auto adji : f.adjFaces) {
      //   auto &adj = qh.faces[adji];
      //   auto c2 = vec::centroid(points[adj.verts[0]], points[adj.verts[1]], points[adj.verts[2]]);
      //   faceLines.insert(faceLines.end(), {
      //      { vec::add(c, vec::mul(f.pln.normal, 0.02f)), CommonColors::Yellow },
      //      { vec::add(c2, vec::mul(adj.pln.normal, 0.01f)), CommonColors::Yellow }
      //   });
      //}
   }

   out.lineModels.push_back(ModelManager::create(faceLines));
   out.pointModels.push_back(ModelManager::create(pointLines));


   auto poly = polys.calculateNormals().expandIndices().createModel(ModelOpts::IncludeColor | ModelOpts::IncludeNormals);
   out.polyModels.push_back(poly);

   auto copoly = copolys.calculateNormals().expandIndices().createModel(ModelOpts::IncludeColor | ModelOpts::IncludeNormals);
   out.polyModels.push_back(copoly);

   return out;
}