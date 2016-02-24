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
   static Matrix scale(Float2 const &v);
   static Matrix translate(Float2 const &v);

   Matrix &operator*=(Matrix const &rhs);
   Matrix operator*(Matrix const &rhs) const;

   float &operator[](size_t index);
   float const &operator[](size_t index) const;

};
