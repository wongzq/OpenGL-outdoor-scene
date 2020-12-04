#version 330 core

uniform vec3 vColor;
uniform vec3 viewPos;
uniform sampler2D ourTexture;
uniform vec3 sunlightPos;
uniform vec3 sunlightColor;

out vec4 fragColor;
in vec3 vNormal;
in vec3 vPos;
in vec2 vTexCoord;
in float textureFlag;
in float sunlightEffect;

void main(void) {
	// diffuse light component calculation
	vec3 normal = normalize(vNormal);

	// sun light
	vec3 sunlightAmbient = vec3(0.3, 0.3, 0.3);
	vec3 sunlightDirection  = normalize(sunlightPos - vPos);
	float dist = distance(sunlightPos, vPos);
	float attenuation = 10.0 / dist;
	vec3 sunlightDiffuse = max(dot(normal, sunlightDirection), 0.0) * (sunlightColor * sunlightEffect);

	vec3 sunlight = sunlightAmbient + sunlightDiffuse;

	fragColor = attenuation * vec4(sunlight * vColor, 1.0);
	fragColor =
		(textureFlag * texture(ourTexture, vTexCoord) * fragColor) +
		((1.0 - textureFlag) * fragColor);
}