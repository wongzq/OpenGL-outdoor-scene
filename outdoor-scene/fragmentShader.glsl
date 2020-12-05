#version 330 core

// uniform locations
uniform vec3 vColor;
uniform vec3 viewPos;
uniform sampler2D ourTexture;
uniform vec3 sunlightPos;
uniform vec3 sunlightColor;
uniform bool useTexture;
uniform bool useFog;
uniform float fogStart;
uniform float fogEnd;

// outputs and inputs
out vec4 fragColor;
in vec3 vNormal;
in vec3 vPos;
in vec2 vTexCoord;
in float textureFlag;
in float sunlightEffect;
in vec4 viewSpace;

// global variables
float fogDensity = 0.1f;
vec4 fogColor = vec4(0.5, 0.5, 0.5, 0.5);

// calculate fog factor of the fragment
float calculateFogFactor(float fogDistance) {
	float result = 0.0;
	result = (fogDistance - fogStart) / (fogEnd - fogStart);

	return clamp(result, 0.0, 1.0);
}

// main function
void main(void) {
	// diffuse light component calculation
	vec3 normal = normalize(vNormal);

	// sunlight ambient
	vec3 sunlightAmbient = vec3(0.5, 0.5, 0.5);
	
	// sunlight diffuse
	vec3 sunlightDirection  = normalize(sunlightPos - vPos);
	float dist = distance(sunlightPos, vPos);
	vec3 sunlightDiffuse = max(dot(normal, sunlightDirection), 0.0) * (sunlightColor * sunlightEffect);

	// combined sunlight
	vec3 sunlight = sunlightAmbient + sunlightDiffuse;

	// final fragment color
	float attenuation = 10.0 / dist;
	fragColor = attenuation * vec4(sunlight * vColor, 1.0);
	fragColor = useTexture && textureFlag == 1.0
		? texture(ourTexture, vTexCoord) * fragColor
		: fragColor;

	// fog
	if (useFog) {
		float fogDistance = length(viewSpace);
		fragColor = mix(fragColor, fogColor, calculateFogFactor(fogDistance));
	}
}