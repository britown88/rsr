#pragma once

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
   Vec3<T> mul(Vec3<T> const &v, T s) {
      Vec3<T> out = { v.x * s, v.y * s, v.z * s };
      return out;
   }

   template<typename T>
   Vec3<T> normal(Vec3<T> const &v) {
      return mul(v, 1.0f / sqrtf(dot(v, v)));
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

   float &operator[](size_t index);
   float const &operator[](size_t index) const;

};