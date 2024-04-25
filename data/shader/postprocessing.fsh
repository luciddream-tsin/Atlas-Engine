#include <globals.hsh>
#include <common/eotf.hsh>
#include <common/random.hsh>

layout (location = 0) out vec4 outColor;

layout (location = 0) in vec2 positionVS;

layout(set = 3, binding = 0) uniform sampler2D hdrTexture;
layout(set = 3, binding = 1) uniform sampler2D bloomFirstTexture;
layout(set = 3, binding = 2) uniform sampler2D bloomSecondTexture;
layout(set = 3, binding = 3) uniform sampler2D bloomThirdTexture;

layout(set = 3, binding = 4) uniform UniformBuffer {
    float exposure;
    float whitePoint;
    float saturation;
    float contrast;
    float filmGrainStrength;
    int bloomPasses;
    float aberrationStrength;
    float aberrationReversed;
    float vignetteOffset;
    float vignettePower;
    float vignetteStrength;
    vec4 vignetteColor;

} Uniforms;

const float gamma = 1.0 / 2.2;

vec3 ACESToneMap(vec3 hdrColor) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((hdrColor*(a*hdrColor+b))/
        (hdrColor*(c*hdrColor+d)+e), 0.0, 1.0);
}

vec3 ToneMap(vec3 hdrColor) {
    
    return vec3(1.0) - exp(-hdrColor);
    
}

float ToneMap(float luminance) {

    return 1.0 - exp(-luminance);

}

vec3 saturate(vec3 color, float factor) {
    const vec3 luma = vec3(0.299, 0.587, 0.114);
    vec3 pixelLuminance = vec3(dot(color, luma));
    return mix(pixelLuminance, color, factor);
}

const mat3 RGBToYCoCgMatrix = mat3(0.25, 0.5, -0.25, 0.5, 0.0, 0.5, 0.25, -0.5, -0.25);
const mat3 YCoCgToRGBMatrix = mat3(1.0, 1.0, 1.0, 1.0, 0.0, -1.0, -1.0, 1.0, -1.0);

vec3 RGBToYCoCg(vec3 RGB) {

    return RGBToYCoCgMatrix * RGB;

}

vec3 YCoCgToRGB(vec3 YCoCg) {

    return YCoCgToRGBMatrix * YCoCg;

}

void main() {
    
    vec2 texCoord = 0.5 * positionVS + 0.5;
    vec3 color = vec3(0.0);
    

    color = texture(hdrTexture, texCoord).rgb;


    // color *= Uniforms.exposure;


    // Apply the tone mapping because we want the colors to be back in
    // normal range
#ifdef HDR
    // Note: Tuned these two eotfs to be perceptually the same. Not sure how it turns out.
    // Haven't tested with Dolby Vision
#ifdef HYBRID_LOG_GAMMA_EOTF
    // Dark regions are getting crushed too much, correct for that
    color = pow(color, vec3(0.9));
    color = Rec709ToRec2020(color);
    color.rgb = InverseHybridLogGammeEotf(color);
#endif

#ifdef PERCEPTUAL_QUANTIZER_EOTF
    color = Rec709ToRec2020(color);
    color = InversePerceptualQuantizerEotf(color);
#endif
    
#else

    color = ToneMap(color);

#ifdef GAMMA_CORRECTION
    color = pow(color, vec3(gamma));
#endif
#endif

    color = clamp(saturate(color, Uniforms.saturation), vec3(0.0), vec3(1.0));

    color = ((color - 0.5) * max(Uniforms.contrast, 0.0)) + 0.5;


    outColor = vec4(color, 1.0);
    
}