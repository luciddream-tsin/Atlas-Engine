#ifndef IRENDERER_H
#define IRENDERER_H

#include "../System.h"
#include "../RenderTarget.h"
#include "../Camera.h"
#include "../Scene.h"
#include "../Window.h"


class IRenderer {

public:
	virtual void Render(Window* window, RenderTarget* target, Camera* camera, Scene* scene, bool masterRenderer = false) = 0;
	virtual ~IRenderer() {}

};

#endif