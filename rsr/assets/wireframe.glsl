layout(std140, binding = 0) uniform uboView{
    mat4 uViewMatrix;
};

#ifdef FRAGMENT
   out vec4 outColor;
   in vec4 vColor;

   void main(){         
      outColor = vColor;
   }
#endif

#ifdef VERTEX
   uniform mat4 uModelMatrix;
   uniform vec4 uColorTransform;

   #ifdef ROTATION
   uniform mat4 uModelRotation;
   #endif

   in vec2 aPosition2;
   in vec3 aPosition3;

   out vec4 vColor;

   void main() {
	  
      vColor = uColorTransform;

      #ifdef POSITION_2D
      vec4 position = vec4(aPosition2, 0, 1);
      #else
	  vec4 position = vec4(aPosition3, 1);
      #endif      

	  mat4 model = uModelMatrix;	  
	  #ifdef ROTATION
	  model *= uModelRotation;
	  #endif

      gl_Position = uViewMatrix * (model * position);;
   }
#endif