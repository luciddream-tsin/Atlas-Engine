layout(location=0)in vec3 vPosition;

// This shader has issues when going from day to night while being in the outer atmosphere, because the integration 
// along the camera ray is somewhat limited due to performance concerns.
// https://atomworld.wordpress.com/2014/12/22/flexible-physical-accurate-atmosphere-scattering-part-1/

out vec3 fPosition;

uniform mat4 vMatrix;
uniform mat4 pMatrix;
uniform vec3 cameraLocation;
uniform vec3 sunDirection;

void main() {
	
    vec4 pos = vec4(vPosition * 6871.0f - vec3(0.0f, 6371.0f, 0.0f), 1.0f);
	fPosition = pos.xyz - cameraLocation;
	gl_Position = (pMatrix * vMatrix * pos).xyww;
	
}