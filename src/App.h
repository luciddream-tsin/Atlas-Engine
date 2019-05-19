#ifndef AE_APP_H
#define AE_APP_H

#include "Engine.h"
#include "input/Mouse.h"
#include "input/Keyboard.h"
#include "input/Controller.h"
#include "input/Touch.h"                  
#include "renderer/gpgpu/OceanSimulation.h"

class Main {

public:
	Main(int argc, char* argv[], Atlas::Window* window);

private:
	void Update(float deltaTime);

	void Render(float deltaTime);

	void Load();

	void DisplayLoadingScreen();

	void SceneSetUp();

	void QuitEventHandler();

	void ControllerDeviceEventHandler(Atlas::Events::ControllerDeviceEvent event);

	Atlas::Window* window;

	Atlas::Input::MouseHandler* mouseHandler;
	Atlas::Input::KeyboardHandler* keyboardHandler;
	Atlas::Input::ControllerHandler* controllerHandler;
	Atlas::Input::TouchHandler* touchHandler;

	Atlas::RenderTarget* renderTarget;
	Atlas::Renderer::MasterRenderer masterRenderer;

	Atlas::Viewport viewport;

	Atlas::Font font;

	Atlas::Camera* camera;
	Atlas::Scene::Scene* scene;

	Atlas::Texture::Cubemap* skybox;

	Atlas::Mesh::Mesh sponzaMesh;
	Atlas::Mesh::Mesh treeMesh;
	Atlas::Mesh::Mesh cubeMesh;

	Atlas::Actor::MovableMeshActor cubeActor;
	Atlas::Actor::StaticMeshActor treeActor;
	Atlas::Actor::StaticMeshActor sponzaActor;

	Atlas::Lighting::DirectionalLight* directionalLight;

	Atlas::Audio::AudioData* audioData;
	Atlas::Audio::AudioStream* audioStream;

	float renderingStart;
	uint32_t frameCount;

	bool quit;
	bool useControllerHandler;

};

#endif