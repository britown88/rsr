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
   smooth in vec4 vColor;
   smooth in vec3 vNormal;
   smooth in vec3 vPosition;

   #ifdef DIFFUSE_TEXTURE
   uniform sampler2D uTexture;
   smooth in vec2 vTexCoords;
   #endif

   uniform samplerCube uSkybox;

   void main(){   
      vec4 color = vColor;

	  #ifdef DIFFUSE_TEXTURE
      color *= texture(uTexture, vTexCoords);
      #endif

	  vec3 I = normalize(vPosition - uCamera.eye);
	  vec3 R = reflect(I, normalize(vNormal));
      //color.rgb *= texture(uSkybox, R).rgb;

	  #ifdef DIFFUSE_LIGHTING
	  vec3 lcolor = color.rgb;
	  color.rgb = vec3(0,0,0);

	  //ambient
	  color.rgb += lcolor * vec3(uLightAmbient, uLightAmbient, uLightAmbient);

	  //diffuse
	  float dotl = max(dot(vNormal, -uLightDirection), 0.0);
	  color.rgb += lcolor * dotl;

	  //specular
	  vec3 reflected = reflect(-uLightDirection, vNormal);
	  float dotspc = pow(max(dot(uCamera.dir, reflected), 0.0), 8.0);

	  color.rgb += lcolor * dotspc;
	  
	  //color = vec4(vec3(1,1,1)*dotspc, 1);
	  //color.rgb = texture(uSkybox, R).rgb;
	  #endif
      
      outColor = color;
   }
#endif

#ifdef VERTEX
   uniform mat4 uModelMatrix;
   uniform vec4 uColorTransform;

   in vec2 aPosition2;
   in vec3 aPosition3;
   in vec4 aColor;
   in vec3 aNormal;

   out vec3 vPosition;
   out vec4 vColor;
   out vec3 vNormal;

   #ifdef DIFFUSE_TEXTURE
   uniform mat4 uTexMatrix;
   in vec2 aTexCoords; 
   smooth out vec2 vTexCoords;
   #endif

   void main() {
	  
      vColor = aColor * uColorTransform;

	  #ifdef DIFFUSE_LIGHTING
	 // vNormal = mat3(transpose(inverse(uModelMatrix))) * aNormal; 
	  vNormal = aNormal; 
	  #endif
      
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

	  vPosition = vec3(uModelMatrix * vec4(position.xyz, 1.0f));
	  
      gl_Position = uViewMatrix * (uModelMatrix * position);;
   }
#endif