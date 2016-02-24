#include "Geom.hpp"
#include <math.h>

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

   float f = 1.0f / tanf((fovy * 0.0174533f) / 2.0f);

   out[0] = f / aspect;
   out[5] = f;
   out[10] = (zFar + zNear) / (zNear - zFar);
   out[11] = -1.0f;
   out[14] = (2 * zFar * zNear) / (zNear - zFar);

   return out;
}

//Matrix frustrum(float left, float right, float bottom, float top, float znear, float zfar)
//{
//   Matrix out = { 0 };
//   float temp, temp2, temp3, temp4;
//   temp = 2.0 * znear;
//   temp2 = right - left;
//   temp3 = top - bottom;
//   temp4 = zfar - znear;
//
//   out[0] = temp / temp2;
//   out[5] = temp / temp3;
//   out[8] = (right + left) / temp2;
//   out[9] = (top + bottom) / temp3;
//   out[10] = (-zfar - znear) / temp4;
//   out[11] = -1.0;
//   out[14] = (-temp * zfar) / temp4;
//
//   return out;
//}
//
//Matrix Matrix::perspective(float fovy, float aspect, float zNear, float zFar)
//{
//   float ymax, xmax;
//   float temp, temp2, temp3, temp4;
//   ymax = zNear * tanf(fovy * 3.14159265f / 360.0);
//   xmax = ymax * aspect;
//   return frustrum(-xmax, xmax, -ymax, ymax, zNear, zFar);
//}

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

float &Matrix::operator[](size_t index) {
   return data[index];
}
float const &Matrix::operator[](size_t index) const {
   return data[index];
}