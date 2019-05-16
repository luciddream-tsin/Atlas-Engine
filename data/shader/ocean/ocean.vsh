layout(location=0)in vec3 vPosition;

layout(binding = 0) uniform sampler2D displacementMap;
layout(binding = 1) uniform sampler2D normalMap;

out vec3 fPosition;
out vec3 fNormal;
out vec2 fTexCoord;
out float fold;

uniform mat4 vMatrix;
uniform mat4 pMatrix;

uniform float choppyScale;
uniform float displacementScale;

const float scale = 0.25;
const float size = 512.0;

void main() {
	
	fPosition = vPosition;
	
	vec2 vTexCoord = vec2(fPosition.x, fPosition.z) / (size * scale);
	
	fPosition.y = texture(displacementMap, vTexCoord).r * displacementScale;
	fPosition.x += texture(displacementMap, vTexCoord).g * choppyScale;
	fPosition.z += texture(displacementMap, vTexCoord).b * choppyScale;
	
	fNormal = vec3(vMatrix * vec4(texture(normalMap, vTexCoord).rgb, 0.0f));
	fold = texture(normalMap, vTexCoord).a;
	
	gl_Position = pMatrix * vMatrix * vec4(fPosition, 1.0);
	
	fTexCoord = vec2(fPosition.x, fPosition.z) / (size * scale);
	fPosition = vec3(vMatrix * vec4(fPosition, 1.0));
	
}