#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 bPos;
layout (location = 4) in vec3 bNormal;
layout (location = 5) in vec2 bTexCoord;

out vec3 vNormal;
out vec3 vPos;
out vec2 vTexCoord;
out float textureFlag;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform int obj;

void main() {
	if (obj == 1) {
		gl_Position = proj * view * model * vec4(aPos, 1.0);
		vPos = vec3(model * vec4(aPos, 1.0));
		vNormal = vec3(model * vec4(aNormal, 0.0));
		vTexCoord = aTexCoord;
		textureFlag = 1.0;
	}
	else if(obj == 2) {
		gl_Position = proj * view * model * vec4(bPos, 1.0);
		vPos = vec3(model * vec4(bPos, 1.0));
		vNormal = vec3(model * vec4(bNormal, 0.0));
		vTexCoord = bTexCoord;
		textureFlag = 1.0;
	}
	else if(obj == 3) {
		gl_Position = proj * view * model * vec4(bPos, 1.0);
		vPos = vec3(model * vec4(bPos, 1.0));
		vNormal = vec3(model * vec4(bNormal, 0.0));
		vTexCoord = bTexCoord;
		textureFlag = 0.0;
	}

}