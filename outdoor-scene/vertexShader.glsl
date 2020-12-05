#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

out vec3 vNormal;
out vec3 vPos;
out vec2 vTexCoord;
out float textureFlag;
out float sunlightEffect;
out vec4 viewSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform int obj;

void main() {
	gl_Position = proj * view * model * vec4(pos, 1.0);
	vPos = vec3(model * vec4(pos, 1.0));
	vNormal = vec3(model * vec4(normal, 0.0));
	viewSpace = view * model * vec4(pos, 1.0);
	
	// water and terrain
	if (obj == 1) {
		textureFlag = 1.0;
		vTexCoord = texCoord;
		sunlightEffect = 1.0;
	}
	// sky
	else if(obj == 2) {
		textureFlag = 1.0;
		vTexCoord = texCoord;
		sunlightEffect = 0.25;
	}
	// GLUT objects
	else if(obj == 3) {
		textureFlag = 0.0;
		sunlightEffect = 1.0;
	}
}