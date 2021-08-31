layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aVertexColor;
layout (location = 4) in vec3 aTangent;
layout (location = 5) in vec3 aBones;
layout (location = 6) in vec3 aBoneWeights;
layout (location = 7) in vec3 aDrawID;

/*
layout (std140) uniform Viewport{
	mat4 CameraMatrix;
	vec3 CameraPos;
};
*/

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;
out vec3 ViewPos;

uniform mat4 MVP;
uniform mat4 Model;
uniform vec3 CameraPos;

void main()
{
	// Vertex position in Eye space
	vec4 ViewSpacePos = Model * vec4(aPosition, 1.0);
	ViewPos = ViewSpacePos.xyz;
	
	// Normal vector
	Normal = mat3(Model) * aNormal;
	
	// Texture coords
	TexCoords = aTexCoords; 

	
	FragPos = vec3(ViewPos) / ViewSpacePos.w;//vec3(Model * vec4(aPosition, 1.0));
  	
	gl_Position = MVP * vec4(aPosition, 1.0); 
}

#if 0

layout (location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
#endif