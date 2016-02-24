#include "Geom.hpp"

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
Matrix Matrix::scale(Float2 const &v) {
   Matrix out = Matrix::identity();
   out[0] = v.x;
   out[5] = v.y;
   return out;
}
Matrix Matrix::translate(Float2 const &v) {
   Matrix out = Matrix::identity();
   out[12] = v.x;
   out[13] = v.y;
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