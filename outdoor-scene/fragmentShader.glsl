#version 330 core

uniform vec3 vColor;
uniform vec3 viewPos;
uniform sampler2D ourTexture;

out vec4 fragColor;
in vec3 vNormal;
in vec3 vPos;
in vec2 vTexCoord;

struct Light {
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

Light light1;

vec3 calcLightDir (Light light, vec3 normal, vec3 viewDir) {
	// ambient light component
	vec3 ambient = light.ambient;

	// diffuse light component
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(light.direction);
	vec3 diffuse = max(dot(norm, lightDir), 0.0) * light.diffuse;

	// specular light component
	vec3 reflectDir = reflect(-lightDir, norm);
	vec3 specular = pow(max(dot(viewDir, reflectDir), 0.0), 128) * light.specular;

	return ambient + diffuse + specular;
}

void main(void) {
	light1.direction = vec3(0.0, 10.0, 0.0);
	light1.ambient = vec3(0.3, 0.3, 0.3);
	light1.diffuse = vec3(0.8, 0.8, 0.8);
	light1.specular = vec3(1.0, 1.0, 1.0);

	vec3 viewDir = normalize(viewPos - vPos);
	vec3 norm = normalize(vNormal);

	vec3 lighting = calcLightDir(light1, norm, viewDir);

	fragColor = vec4(lighting * vColor, 1.0);
	fragColor = texture(ourTexture, vTexCoord) * fragColor;
}