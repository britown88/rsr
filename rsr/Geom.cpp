#include "Geom.hpp"
#include "Defs.hpp"
#include <math.h>
#include <algorithm>

Matrix Matrix::identity() {
   Matrix out = { 0 };
   out[0] = 1.0f;
   out[5] = 1.0f;
   out[10] = 1.0f;
   out[15] = 1.0f;
   return out;
}
Matrix Matrix::ortho(float left, float right, float bottom, float top, float near, float far) {
   Matrix out = Matrix::identity();

   out[1] = out[2] = out[3] = out[4] = out[6] =
   out[7] = out[8] = out[9] = out[11] = 0.0f;

   out[0] = 2.0f / (right - left);
   out[5] = 2.0f / (top - bottom);
   out[10] = -2.0f / (far - near);
   out[15] = 1.0f;

   out[12] = -((right + left) / (right - left));
   out[13] = -((top + bottom) / (top - bottom));
   out[14] = -((far + near) / (far - near));
   return out;
}
Matrix Matrix::perspective(float fovy, float aspect, float zNear, float zFar) {
   Matrix out = { 0 };

   float f = 1.0f / tanf((fovy * DEG2RAD) / 2.0f);

   out[0] = f / aspect;
   out[5] = f;
   out[10] = (zFar + zNear) / (zNear - zFar);
   out[11] = -1.0f;
   out[14] = (2 * zFar * zNear) / (zNear - zFar);

   return out;
}

Matrix Matrix::fromBasis(Float3 const &v1, Float3 const &v2, Float3 const &v3) {
   Matrix out = identity();

   out[0] = v1.x; out[1] = v1.y; out[2] = v1.z;
   out[4] = v2.x; out[5] = v2.y; out[6] = v2.z;
   out[8] = v3.x; out[9] = v3.y; out[10] = v3.z;

   return out;
}

Matrix Matrix::lookAt(Float3 const &eye, Float3 const &center, Float3 const &up) {
   Matrix out = { 0 };
   Float3 n = vec::normal(vec::sub(eye, center));
   Float3 u = vec::normal(vec::cross(up, n));
   Float3 v = vec::normal(vec::cross(n, u));

   out[0] = u.x;
   out[4] = u.y;
   out[8] = u.z;

   out[1] = v.x;
   out[5] = v.y;
   out[9] = v.z;

   out[2] = n.x;
   out[6] = n.y;
   out[10] = n.z;

   out[12] = -vec::dot(eye, u);
   out[13] = -vec::dot(eye, v);
   out[14] = -vec::dot(eye, n);

   out[15] = 1.0f;

   return out;
}

Matrix Matrix::scale2f(Float2 const &v) {
   Matrix out = Matrix::identity();
   out[0] = v.x;
   out[5] = v.y;
   return out;
}
Matrix Matrix::scale3f(Float3 const &v) {
   Matrix out = Matrix::identity();
   out[0] = v.x;
   out[5] = v.y;
   out[10] = v.z;
   return out;
}
Matrix Matrix::translate2f(Float2 const &v) {
   Matrix out = Matrix::identity();
   out[12] = v.x;
   out[13] = v.y;
   return out;
}
Matrix Matrix::translate3f(Float3 const &v) {
   Matrix out = Matrix::identity();
   out[12] = v.x;
   out[13] = v.y;
   out[14] = v.z;
   return out;
}
Matrix Matrix::invert(Matrix const &m) {
   Matrix inv;
   double det;
   int i;

   inv[0] = m[5] * m[10] * m[15] -
      m[5] * m[11] * m[14] -
      m[9] * m[6] * m[15] +
      m[9] * m[7] * m[14] +
      m[13] * m[6] * m[11] -
      m[13] * m[7] * m[10];

   inv[4] = -m[4] * m[10] * m[15] +
      m[4] * m[11] * m[14] +
      m[8] * m[6] * m[15] -
      m[8] * m[7] * m[14] -
      m[12] * m[6] * m[11] +
      m[12] * m[7] * m[10];

   inv[8] = m[4] * m[9] * m[15] -
      m[4] * m[11] * m[13] -
      m[8] * m[5] * m[15] +
      m[8] * m[7] * m[13] +
      m[12] * m[5] * m[11] -
      m[12] * m[7] * m[9];

   inv[12] = -m[4] * m[9] * m[14] +
      m[4] * m[10] * m[13] +
      m[8] * m[5] * m[14] -
      m[8] * m[6] * m[13] -
      m[12] * m[5] * m[10] +
      m[12] * m[6] * m[9];

   inv[1] = -m[1] * m[10] * m[15] +
      m[1] * m[11] * m[14] +
      m[9] * m[2] * m[15] -
      m[9] * m[3] * m[14] -
      m[13] * m[2] * m[11] +
      m[13] * m[3] * m[10];

   inv[5] = m[0] * m[10] * m[15] -
      m[0] * m[11] * m[14] -
      m[8] * m[2] * m[15] +
      m[8] * m[3] * m[14] +
      m[12] * m[2] * m[11] -
      m[12] * m[3] * m[10];

   inv[9] = -m[0] * m[9] * m[15] +
      m[0] * m[11] * m[13] +
      m[8] * m[1] * m[15] -
      m[8] * m[3] * m[13] -
      m[12] * m[1] * m[11] +
      m[12] * m[3] * m[9];

   inv[13] = m[0] * m[9] * m[14] -
      m[0] * m[10] * m[13] -
      m[8] * m[1] * m[14] +
      m[8] * m[2] * m[13] +
      m[12] * m[1] * m[10] -
      m[12] * m[2] * m[9];

   inv[2] = m[1] * m[6] * m[15] -
      m[1] * m[7] * m[14] -
      m[5] * m[2] * m[15] +
      m[5] * m[3] * m[14] +
      m[13] * m[2] * m[7] -
      m[13] * m[3] * m[6];

   inv[6] = -m[0] * m[6] * m[15] +
      m[0] * m[7] * m[14] +
      m[4] * m[2] * m[15] -
      m[4] * m[3] * m[14] -
      m[12] * m[2] * m[7] +
      m[12] * m[3] * m[6];

   inv[10] = m[0] * m[5] * m[15] -
      m[0] * m[7] * m[13] -
      m[4] * m[1] * m[15] +
      m[4] * m[3] * m[13] +
      m[12] * m[1] * m[7] -
      m[12] * m[3] * m[5];

   inv[14] = -m[0] * m[5] * m[14] +
      m[0] * m[6] * m[13] +
      m[4] * m[1] * m[14] -
      m[4] * m[2] * m[13] -
      m[12] * m[1] * m[6] +
      m[12] * m[2] * m[5];

   inv[3] = -m[1] * m[6] * m[11] +
      m[1] * m[7] * m[10] +
      m[5] * m[2] * m[11] -
      m[5] * m[3] * m[10] -
      m[9] * m[2] * m[7] +
      m[9] * m[3] * m[6];

   inv[7] = m[0] * m[6] * m[11] -
      m[0] * m[7] * m[10] -
      m[4] * m[2] * m[11] +
      m[4] * m[3] * m[10] +
      m[8] * m[2] * m[7] -
      m[8] * m[3] * m[6];

   inv[11] = -m[0] * m[5] * m[11] +
      m[0] * m[7] * m[9] +
      m[4] * m[1] * m[11] -
      m[4] * m[3] * m[9] -
      m[8] * m[1] * m[7] +
      m[8] * m[3] * m[5];

   inv[15] = m[0] * m[5] * m[10] -
      m[0] * m[6] * m[9] -
      m[4] * m[1] * m[10] +
      m[4] * m[2] * m[9] +
      m[8] * m[1] * m[6] -
      m[8] * m[2] * m[5];

   det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
   det = 1.0 / det;

   Matrix out = { 0 };
   if (det == 0) {
      return out;
   }

   for (i = 0; i < 16; i++) {
      out[i] = inv[i] * det;
   }

   return out;
}

Matrix Matrix::transpose(Matrix const &m) {
   Matrix out;

   out[0] = m[0];
   out[1] = m[4];
   out[2] = m[8];
   out[3] = m[12];
   out[4] = m[1];
   out[5] = m[5];
   out[6] = m[9];
   out[7] = m[13];
   out[8] = m[2];
   out[9] = m[6];
   out[10] = m[10];
   out[11] = m[14];
   out[12] = m[3];
   out[13] = m[7];
   out[14] = m[11];
   out[15] = m[15];

   return out;
}

Matrix &Matrix::operator*=(Matrix const &rhs) {
   *this = *this * rhs;
   return *this;
}
Matrix Matrix::operator*(Matrix const &rhs) const {
   Matrix out;
   int x, y, i;
   for (y = 0; y < 4; ++y)
   {
      float v1[4] = { this->data[y], this->data[y + 4], this->data[y + 8], this->data[y + 12] };

      for (x = 0; x < 4; ++x)
      {
         const float *v2 = &(rhs[x * 4]);

         float v = 0.0f;
         for (i = 0; i < 4; ++i)
            v += v1[i] * v2[i];

         out[x * 4 + y] = v;
      }
   }

   return out;
}

Float3 Matrix::operator*(Float3 const &rhs) const {
   Float3 out;
   
   out.x = vec::dot(rhs, { data[0] , data[4] , data[8] });
   out.y = vec::dot(rhs, { data[1] , data[5] , data[9] });
   out.z = vec::dot(rhs, { data[2] , data[6] , data[10] });

   return out;
}

float &Matrix::operator[](size_t index) {
   return data[index];
}
float const &Matrix::operator[](size_t index) const {
   return data[index];
}

Quaternion Quaternion::fromAxisAngle(Float3 axis, float angleInDegrees) {
   float halfAngle = (angleInDegrees*DEG2RAD) / 2.0f;
   float sinHalfAngle = (float)sin(halfAngle);

   return { 
      { 
         axis.x * sinHalfAngle,
         axis.y * sinHalfAngle,
         axis.z * sinHalfAngle
      },
      (float)cos(halfAngle)
   };
}

Quaternion Quaternion::unit() {
   return { {0.0f, 0.0f, 0.0f}, 1.0f };
}

Matrix Quaternion::toMatrix() {
   float &qx = xyz.x;
   float &qy = xyz.y;
   float &qz = xyz.z;
   float &qw = w;

   return {
      1 - 2 * qy*qy - 2 * qz*qz, 2 * qx*qy - 2 * qz*qw,     2 * qx*qz + 2 * qy*qw,     0.0f,
      2 * qx*qy + 2 * qz*qw,     1 - 2 * qx*qx - 2 * qz*qz, 2 * qy*qz - 2 * qx*qw,     0.0f,
      2 * qx*qz - 2 * qy*qw,     2 * qy*qz + 2 * qx*qw,     1 - 2 * qx*qx - 2 * qy*qy, 0.0f,
      0.0f,                      0.0f,                      0.0f,                      1.0f
   };
}

Float3 Quaternion::rotate(Float3 &pt)  {
   float x1 = xyz.y*pt.z - xyz.z*pt.y;
   float y1 = xyz.z*pt.x - xyz.x*pt.z;
   float z1 = xyz.x*pt.y - xyz.y*pt.x;

   float x2 = w*x1 + xyz.y*z1 - xyz.z*y1;
   float y2 = w*y1 + xyz.z*x1 - xyz.x*z1;
   float z2 = w*z1 + xyz.x*y1 - xyz.y*x1;

   return { pt.x + 2.0f*x2 , pt.y + 2.0f*y2, pt.z + 2.0f*z2 };
}

float linterp(float f1, float f2, float t) {
   return f1 + (f2 - f1) * t;
}

float cinterp(float y0, float y1, float y2, float y3, float mu) {
   float a0, a1, a2, a3, mu2;

   mu2 = mu*mu;
   a0 = y3 - y2 - y0 + y1;
   a1 = y0 - y1 - a0;
   a2 = y2 - y0;
   a3 = y1;

   return(a0*mu*mu2 + a1*mu2 + a2*mu + a3);

}

bool Plane::behind(Plane const &p, Float3 const &point) {
   return vec::dot(vec::normal(vec::sub(point, p.orig)), p.normal) < 0;
}

Plane Plane::fromFace(Float3 const &v1, Float3 const &v2, Float3 const &v3) {
   return {v1, vec::faceNormal(v1, v2, v3)};
}

Float3 vec::centroid(Float3 const &v1, Float3 const &v2, Float3 const &v3) {
   return {
      (v1.x + v2.x + v3.x) / 3.0f,
      (v1.y + v2.y + v3.y) / 3.0f,
      (v1.z + v2.z + v3.z) / 3.0f
   };
}

Float3 vec::cross(Float3 const &v1, Float3 const &v2) {
   Float3 out = {
      (v1.y * v2.z) - (v1.z*v2.y),
      (v1.z*v2.x) - (v1.x*v2.z),
      (v1.x*v2.y) - (v1.y*v2.x)
   };
   return out;
}

float vec::dot(Float3 const &v1, Float3 const &v2) {
   return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

Float3 vec::normal(Float3 const &v) {
   return mul(v, 1.0f / sqrtf(dot(v, v)));
}

float vec::lensq(Float3 const &v) {
   return dot(v, v);
}

float vec::len(Float3 const &v) {
   return sqrtf(lensq(v));
}

Float3 vec::faceNormal(Float3 const &v1, Float3 const &v2, Float3 const &v3) {
   return normal(cross(sub(v2, v1), sub(v3, v1)));
}

float vec::distPoint2Line(Float3 const &p, Float3 const &v1, Float3 const &v2) {
   return len(cross(sub(p, v1), sub(p, v2))) / len(sub(v2, v1));
}
float vec::distPoint2LineSegment(Float3 const &p, Float3 const &v1, Float3 const &v2) {
   float t = std::min(1.0f, std::max(0.0f, -dot(sub(v1, p), sub(v2, v1)) / lensq(sub(v2, v1))));
   
   Float3 vo;
   vo.x = (v1.x - p.x) + (v2.x - v1.x) * t;
   vo.y = (v1.y - p.y) + (v2.y - v1.y) * t;
   vo.z = (v1.z - p.z) + (v2.z - v1.z) * t;
   return len(vo);
}
float vec::distPoint2Plane(Float3 const &pt, Plane const &pl) {
   auto projected = projectPoint2Plane(pt, pl.orig, pl.normal);
   return len(sub(pt, projected));
}

//calculates the normal
float vec::distPoint2Face(Float3 const &p, Float3 const &v1, Float3 const &v2, Float3 const &v3) {
   return distPoint2FaceWithNormal(p, v1, v2, v3, faceNormal(v1, v2, v3));
}

//pass in precalculated normal
float vec::distPoint2FaceWithNormal(Float3 const &p, Float3 const &v1, Float3 const &v2, Float3 const &v3, Float3 const &n) {
   auto projected = projectPoint2Plane(p, v1, n);

   if (pointInTriangle(projected, v1, v2, v3)) {
      return len(sub(p, projected));
   }
   else {
      float d1 = distPoint2LineSegment(p, v1, v2);
      float d2 = distPoint2LineSegment(p, v1, v3);
      float d3 = distPoint2LineSegment(p, v2, v3);
      return std::min(d1, std::min(d2, d3));
   }
}

Float3 vec::projectPoint2Plane(Float3 const &p, Float3 const &plo, Float3 const &pln) {
   return sub(p, mul(pln, dot(sub(p, plo), pln)));;
}

static bool sameSide(Float3 const &p1, Float3 const &p2, Float3 const &a, Float3 const &b) {
   auto c1 = vec::cross(vec::sub(b, a), vec::sub(p1, a));
   auto c2 = vec::cross(vec::sub(b, a), vec::sub(p2, a));
   return vec::dot(c1, c2) >= 0.0f;
}

bool vec::pointInTriangle(Float3 const &p, Float3 const &v1, Float3 const &v2, Float3 const &v3) {
   return sameSide(p, v1, v2, v3) && sameSide(p, v2, v1, v3) && sameSide(p, v3, v1, v2);
}


Spherical vec::toSpherical(Float3 const &v) {
   Spherical out;

   out.r = len(v);
   out.dip = asinf(v.y / out.r) * RAD2DEG;
   out.azm = atan2f(v.z, v.x) * RAD2DEG;

   return out;
}

