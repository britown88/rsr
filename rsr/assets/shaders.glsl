#ifdef FRAGMENT
   out vec4 outColor;
   smooth in vec4 vColor;

   #ifdef DIFFUSE_TEXTURE
   uniform sampler2D uTexture;
   smooth in vec2 vTexCoords;
   #endif

   void main(){   
      vec4 color = vColor;
      
      #ifdef DIFFUSE_TEXTURE
      color *= texture(uTexture, vTexCoords);
      #endif
      
      outColor = color;
   }
#endif

#ifdef VERTEX
   layout(std140, binding = 0) uniform uboView{
      mat4 uViewMatrix;
   };

   uniform mat4 uModelMatrix;
   uniform vec4 uColorTransform;

   in vec2 aPosition2;
   in vec3 aPosition3;
   in vec4 aColor;

   out vec4 vColor;

   #ifdef DIFFUSE_TEXTURE
   uniform mat4 uTexMatrix;
   in vec2 aTexCoords; 
   smooth out vec2 vTexCoords;
   #endif

   void main() {
      vColor = aColor * uColorTransform;
      
      #ifdef DIFFUSE_TEXTURE
      vec4 coord = vec4(aTexCoords, 0.0, 1.0);
      coord = uTexMatrix * coord;
      vTexCoords = coord.xy;
      #endif

      #ifdef POSITION_2D
      vec4 position = vec4(aPosition2, 0, 1);
      #else
	  vec4 position = vec4(aPosition3, 1);
      #endif      
      
      gl_Position = uViewMatrix * (uModelMatrix * position);
   }
#endif