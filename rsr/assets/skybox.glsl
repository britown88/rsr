struct Camera{
	vec3 eye;
	vec3 center;
	vec3 up;
	vec3 dir;
	mat4 persp;
};
layout(std140, binding = 0) uniform uboView{
    mat4 uViewMatrix;
	vec3 uLightDirection;
	float uLightAmbient;
	Camera uCamera;
};

#ifdef FRAGMENT
   out vec4 outColor;

   uniform samplerCube uSkybox;
   in vec3 vTexCoords;

   void main(){        
     outColor = texture(uSkybox, vTexCoords);
	 //outColor = vec4(vTexCoords, 1);
   }
#endif

#ifdef VERTEX

   in vec3 aPosition3;
   out vec3 vTexCoords;

   void main() {
      gl_Position = uViewMatrix * vec4(aPosition3, 1.0);
	  vTexCoords = aPosition3;
   }
#endif