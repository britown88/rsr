#pragma once

#include <vector>

float linterp(float f1, float f2, float t);
float cinterp(float f1, float f2, float f3, float f4, float t);

template<typename T>
struct Vec2 {
   T x, y;
};

template<typename T>
struct Vec3 {
   T x, y, z;
};

typedef Vec2<int> Int2;
typedef Vec2<float> Float2;
typedef Vec3<int> Int3;
typedef Vec3<float> Float3;

namespace vec
{
   template<typename T>
   Vec3<T> cross(Vec3<T> const &v1, Vec3<T> const &v2) {
      Vec3<T> out = {
         (v1.y * v2.z) - (v1.z*v2.y),
            (v1.z*v2.x) - (v1.x*v2.z),
            (v1.x*v2.y) - (v1.y*v2.x)
      };
      return out;
   }

   template<typename T>
   T dot(Vec3<T> const &v1, Vec3<T> const &v2) {
      return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
   }

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

   template<typename T>
   Vec3<T> normal(Vec3<T> const &v) {
      return mul(v, 1.0f / sqrtf(dot(v, v)));
   }

   template<typename T>
   float len(Vec3<T> const &v) {
      return sqrtf(dot(v, v));
   }
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

struct Plane {
   Float3 pos, normal;

   static bool above(Plane const &p, Float3 const &point) {
      return vec::dot(vec::sub(p.pos, point), p.normal) > 0;
   }
};

struct ConvexHull {
   std::vector<Float3> vertices;
   std::vector<Plane> planes;
};

ConvexHull quickHull(std::vector<Float3> &pointCloud);

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

   Matrix &operator*=(Matrix const &rhs);
   Matrix operator*(Matrix const &rhs) const;

   Float3 operator*(Float3 const &rhs) const;

   float &operator[](size_t index);
   float const &operator[](size_t index) const;

};

struct Quaternion {
   Float3 xyz;
   float w;

   static Quaternion fromAxisAngle(Float3 axis, float angle);
   static Quaternion unit();
   Matrix toMatrix();
   Float3 rotate(Float3 &pt);
};






