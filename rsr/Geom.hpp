#pragma once

#include <vector>

static const float PI = 3.14159265359f;
static const float RAD2DEG = 180.0f / PI;
static const float DEG2RAD = PI / 180.0f;

float linterp(float f1, float f2, float t);
float cinterp(float f1, float f2, float f3, float f4, float t);

template<typename T>
struct Vec2 {
   T x, y;
};

template<typename T>
struct Vec3 {
   T x, y, z;
   bool operator==(Vec3<T> const &rhs) {
      return x == rhs.x && y == rhs.y && z == rhs.z;
   }
};

typedef Vec2<int> Int2;
typedef Vec2<float> Float2;
typedef Vec3<int> Int3;
typedef Vec3<float> Float3;

struct Spherical {
   float r, //radial distance
      dip, //zenith angle (phi) in degrees
      azm; //azimuth angle (theta) in degrees

   static Float3 toCartesian(Spherical const &s) {
      float phi = s.dip * DEG2RAD;
      float theta = s.azm * DEG2RAD;

      float cosPhi = cosf(phi);

      //y-z swap
      return{
         s.r * cosPhi * cos(theta),
         s.r * sin(phi),
         s.r * cosPhi * sin(theta)
         
      };
   }
};

struct Plane {
   Float3 orig, normal;
   static bool behind(Plane const &p, Float3 const &point);
   static Plane fromFace(Float3 const &v1, Float3 const &v2, Float3 const &v3);
};

struct ConvexHull {
   std::vector<Float3> vertices;
   std::vector<Plane> planes;
};

std::vector<Float3> quickHull(std::vector<Float3> &pointCloud);

class Model;
struct QuickHullTestModels {
   //
   std::vector<Model*> lineModels;
   std::vector<Model*> pointModels;
   std::vector<Model*> polyModels;

};

QuickHullTestModels quickHullTest(std::vector<Float3> &pointCloud, int iterCount);

namespace vec
{
   template<typename T>
   Vec3<T> add(Vec3<T> const &v1, Vec3<T> const &v2) {
      Vec3<T> out = {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
      return out;
   }
   template<typename T>
   Vec3<T> sub(Vec3<T> const &v1, Vec3<T> const &v2) {
      Vec3<T> out = { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
      return out;
   }
   template<typename T>
   Vec3<T> negate(Vec3<T> const &v) {
      Vec3<T> out = { -v.x, -v.y, -v.z };
      return out;
   }
   template<typename T>
   Vec3<T> mul(Vec3<T> const &v, T s) {
      Vec3<T> out = { v.x * s, v.y * s, v.z * s };
      return out;
   }
   template<typename T>
   Vec3<T> linterp(Vec3<T> const &v1, Vec3<T> const &v2, float t) {
      Vec3<T> out = { 
         ::linterp(v1.x, v2.x, t),
         ::linterp(v1.y, v2.y, t),
         ::linterp(v1.z, v2.z, t),
         };
      return out;
   }
   template<typename T>
   Vec3<T> cinterp(Vec3<T> const &v1, Vec3<T> const &v2, Vec3<T> const &v3, Vec3<T> const &v4, float t) {
      Vec3<T> out = {
         ::cinterp(v1.x, v2.x, v3.x, v4.x, t),
         ::cinterp(v1.y, v2.y, v3.y, v4.y, t),
         ::cinterp(v1.z, v2.z, v3.z, v4.z, t),
      };
      return out;
   }
   
   Float3 cross(Float3 const &v1, Float3 const &v2);
   float dot(Float3 const &v1, Float3 const &v2);
   Float3 normal(Float3 const &v);

   float lensq(Float3 const &v);
   float len(Float3 const &v);

   Float3 faceNormal(Float3 const &v1, Float3 const &v2, Float3 const &v3);

   Float3 centroid(Float3 const &v1, Float3 const &v2, Float3 const &v3);

   float distPoint2Line(Float3 const &p, Float3 const &v1, Float3 const &v2);
   float distPoint2LineSegment(Float3 const &p, Float3 const &v1, Float3 const &v2);
   float distPoint2Plane(Float3 const &pt, Plane const &pl);

   //calculates the normal
   float distPoint2Face(Float3 const &p, Float3 const &v1, Float3 const &v2, Float3 const &v3);

   //pass in precalculated normal
   float distPoint2FaceWithNormal(Float3 const &p, Float3 const &v1, Float3 const &v2, Float3 const &v3, Float3 const &n);

   Float3 projectPoint2Plane(Float3 const &p, Float3 const &plo, Float3 const &pln);
   bool pointInTriangle(Float3 const &p, Float3 const &v1, Float3 const &v2, Float3 const &v3);

   Spherical toSpherical(Float3 const &v);
}

template<typename T>
struct Rect {
   Vec2<T> top, bot;
};

typedef Rect<int> Recti;
typedef Rect<float> Rectf;

template<typename T>
T width(Rect<T> const &r) { return r.bot.x - r.top.x; }

template<typename T>
T height(Rect<T> const &r) { return r.bot.y - r.top.y; }



struct  Matrix {
   float data[16];
public:

   static Matrix identity();
   static Matrix ortho(float left, float right, float bottom, float top, float near, float far);
   static Matrix perspective(float fovy, float aspect, float zNear, float zFar);
   static Matrix lookAt(Float3 const &eye, Float3 const &center, Float3 const &up);
   static Matrix scale2f(Float2 const &v);
   static Matrix scale3f(Float3 const &v);
   static Matrix translate2f(Float2 const &v);
   static Matrix translate3f(Float3 const &v);
   static Matrix fromBasis(Float3 const &v1, Float3 const &v2, Float3 const &v3);
   static Matrix invert(Matrix const &m);
   static Matrix transpose(Matrix const &m);


   Matrix &operator*=(Matrix const &rhs);
   Matrix operator*(Matrix const &rhs) const;

   Float3 operator*(Float3 const &rhs) const;

   float &operator[](size_t index);
   float const &operator[](size_t index) const;

};

struct Quaternion {
   Float3 xyz;
   float w;

   static Quaternion fromAxisAngle(Float3 axis, float angleInDegrees);
   static Quaternion unit();
   Matrix toMatrix();
   Float3 rotate(Float3 &pt);
};






