#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include "../System.h"
#include "IRenderer.h"

#include "../Font.h"

class TextRenderer : public IRenderer {

public:
	TextRenderer(string vertexSource, string fragmentSource);

	virtual void Render(Window* window, RenderTarget* target, Camera* camera, Scene* scene, bool masterRenderer = false);

	void Render(Window* window, Font* font, string text, int32_t x, int32_t y, vec4 color, float scale = 1.0f, Framebuffer* framebuffer = nullptr);

	void RenderOutlined(Window* window, Font* font, string text, int32_t x, int32_t y, vec4 color, float scale = 1.0f, Framebuffer* framebuffer = nullptr);

private:
	void GetUniforms();

	vec3* CalculateCharacterInstances(Font* font, string text, int32_t* characterCount);

	uint32_t vao;
	uint32_t vbo;

	uint32_t vboLength;

	Shader* shader;

	Uniform* glyphsTexture;
	Uniform* projectionMatrix;
	Uniform* characterScales;
	Uniform* characterSizes;
	Uniform* characterOffsets;
	Uniform* textOffset;
	Uniform* textScale;
	Uniform* textColor;
	Uniform* pixelDistanceScale;
	Uniform* edgeValue;

};


#endif