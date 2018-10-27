#include "../structures"
#include "../shadow"

#include "convert"

out vec3 fragColor;

uniform sampler2D textures[5];

in vec2 fTexCoord;
uniform Light light;

uniform mat4 lightSpaceMatrix;//Is pre-multiplied with the inverseViewMatrix
uniform mat4 ivMatrix;

uniform float occlusionStrength;

void main() {
	
	float depth = texture(textures[3], fTexCoord).r;
	
	if (depth == 1.0f)
		discard;
	
	vec3 fragPos = ConvertDepthToViewSpace(depth, fTexCoord);	
	vec3 normal = normalize(2.0f * texture(textures[1], fTexCoord).rgb - 1.0f);
	
	vec3 surfaceColor = texture(textures[0], fTexCoord).rgb;
	vec2 additional = texture(textures[2], fTexCoord).rg;
	
	//Material properties
	float specularIntensity = additional.r;
	float specularHardness = additional.g;
	
	float shadowFactor = 1.0f;
	
#if defined(SHADOWS)
	vec4 shadowCoords = lightSpaceMatrix * vec4(fragPos, 1.0f);
	shadowCoords.z -= light.shadow.bias;
	shadowCoords.xyz /= shadowCoords.w;
	shadowCoords.w = clamp((length(fragPos) - light.shadow.distance) * 0.1f, 0.0f, 1.0f);
	
	vec3 modelCoords = vec3(ivMatrix * vec4(fragPos, 1.0f));
	
	shadowFactor = CalculateShadow(light, modelCoords, shadowCoords); 
#endif

	vec3 specular = vec3(0.0f);
	vec3 diffuse = vec3(1.0f);
	vec3 ambient = vec3(light.ambient * surfaceColor);
	
	float occlusionFactor = 1.0f;
		
	vec3 viewDir = normalize(-fragPos);
	vec3 lightDir = normalize(light.position - fragPos);
	
#ifdef SSAO
	occlusionFactor = pow(texture(textures[4], fTexCoord).r, occlusionStrength);
	ambient *= occlusionFactor;
#endif
	
	diffuse = max((dot(normal, lightDir) * light.color) * shadowFactor, ambient * occlusionFactor)
		* surfaceColor;		
	
	if(specularIntensity > 0.0f && shadowFactor > 0.5f) {
		
		vec3 halfwayDir = normalize(lightDir + viewDir);  
		float dampedFactor = pow(max(dot(normal, halfwayDir), 0.0f), specularHardness);
		specular = light.color * dampedFactor * specularIntensity;
	
	}
	
	if(shadowFactor < 0.5f)
		fragColor = diffuse + ambient;
	else
		fragColor = diffuse + specular + ambient;

}