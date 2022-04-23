#include "App.h"

#include <chrono>
#include <thread>

const std::string Atlas::EngineInstance::assetDirectory = "../../data";
const std::string Atlas::EngineInstance::shaderDirectory = "shader";

void App::LoadContent() {

	UnlockFramerate();

	renderTarget = new Atlas::RenderTarget(1920, 1080);
	pathTraceTarget = Atlas::Renderer::PathTracerRenderTarget(1920, 1080);

	auto icon = Atlas::Texture::Texture2D("icon.png");
	window.SetIcon(&icon);
	window.Update();

	font = Atlas::Font("font/roboto.ttf", 44, 10);

	DisplayLoadingScreen();

	camera = Atlas::Camera(47.0f, 2.0f, 1.0f, 400.0f,
		vec3(30.0f, 25.0f, 0.0f), vec2(-3.14f / 2.0f, 0.0f));

	scene = Atlas::Scene::Scene(vec3(-2048.0f), vec3(2048.0f));

	mouseHandler = Atlas::Input::MouseHandler(&camera, 1.5f, 6.0f);
	keyboardHandler = Atlas::Input::KeyboardHandler(&camera, 7.0f, 6.0f);

	Atlas::Events::EventManager::KeyboardEventDelegate.Subscribe(
		[this](Atlas::Events::KeyboardEvent event) {
			if (event.keycode == AE_KEY_ESCAPE) {
				Exit();
			}
			if (event.keycode == AE_KEY_F11 && event.state == AE_BUTTON_RELEASED) {
				renderUI = !renderUI;
			}
			if (event.keycode == AE_KEY_LSHIFT && event.state == AE_BUTTON_PRESSED) {
				keyboardHandler.speed = cameraSpeed * 4.0f;
			}
			if (event.keycode == AE_KEY_LSHIFT && event.state == AE_BUTTON_RELEASED) {
				keyboardHandler.speed = cameraSpeed;
			}
		});
	
	directionalLight = Atlas::Lighting::DirectionalLight(AE_MOVABLE_LIGHT);
	directionalLight.direction = vec3(0.0f, -1.0f, 1.0f);
	directionalLight.color = vec3(253, 194, 109) / 255.0f;
	mat4 orthoProjection = glm::ortho(-100.0f, 100.0f, -70.0f, 120.0f, -120.0f, 120.0f);
	directionalLight.AddShadow(200.0f, 3.0f, 4096, vec3(0.0f), orthoProjection);
	directionalLight.AddVolumetric(10, 0.28f);
	scene.Add(&directionalLight);

	scene.ssao = new Atlas::Lighting::SSAO(32);

	scene.fog = new Atlas::Lighting::Fog();
	scene.fog->enable = true;
	scene.fog->density = 0.0002f;
	scene.fog->heightFalloff = 0.0284f;
	scene.fog->height = 0.0f;
	scene.fog->scatteringAnisotropy = 0.0f;

	scene.postProcessing.taa = Atlas::PostProcessing::TAA(0.99f);
	scene.postProcessing.sharpen.enable = true;
	scene.postProcessing.sharpen.factor = 0.15f;

	LoadScene();

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	imguiWrapper = ImguiWrapper(&window, &context);

	io.Fonts->AddFontFromFileTTF(
		Atlas::Loader::AssetLoader::GetFullPath("font/roboto.ttf").c_str(),
		20.0f);

	ImGui::StyleColorsDark();	

}

void App::UnloadContent() {



}

void App::Update(float deltaTime) {

	ImGuiIO& io = ImGui::GetIO();

	imguiWrapper.Update(&window, deltaTime);

	if (io.WantCaptureMouse) {
		mouseHandler.lock = true;
	}
	else {
		mouseHandler.lock = false;
	}

	mouseHandler.Update(&camera, deltaTime);
	keyboardHandler.Update(&camera, deltaTime);

	camera.UpdateView();
	camera.UpdateProjection();

	scene.Update(&camera, deltaTime);

}

void App::Render(float deltaTime) {

	static bool firstFrame = true;
	static bool animateLight = false;
	static bool pathTrace = true;
	static bool showAo = false;
	static bool slowMode = false;
	
	window.Clear();

	if (animateLight) directionalLight.direction = vec3(0.0f, -1.0f, sin(Atlas::Clock::Get() / 10.0f));	
	
	if (pathTrace) {
		viewport.Set(0, 0, pathTraceTarget.GetWidth(), pathTraceTarget.GetHeight());
		pathTracingRenderer.Render(&viewport, &pathTraceTarget, ivec2(1, 1), &camera, &scene);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT |
			GL_SHADER_STORAGE_BARRIER_BIT);

		viewport.Set(0, 0, window.GetWidth(), window.GetHeight());
		masterRenderer.RenderTexture(&viewport, &pathTraceTarget.texture, 0.0f, 0.0f,
			(float)viewport.width, (float)viewport.height);
	}
	else {
		viewport.Set(0, 0, window.GetWidth(), window.GetHeight());
		masterRenderer.RenderScene(&viewport, renderTarget, &camera, &scene);

		if (showAo) {
			masterRenderer.RenderTexture(&viewport, &renderTarget->ssaoTexture, 0.0f, 0.0f,
				viewport.width, viewport.height);
		}
	}
	
	float averageFramerate = Atlas::Clock::GetAverage();

	// ImGui rendering
	if (renderUI) {
		ImGui::NewFrame();

		auto& light = directionalLight;
		auto& volume = scene.irradianceVolume;
		auto& ssao = scene.ssao;
		auto& fog = scene.fog;

		bool openSceneNotFoundPopup = false;

		auto floatToString = [](auto number) -> std::string {
			auto str = std::to_string(number);
			auto pos = str.find(".");
			if (pos != std::string::npos)
				return str.substr(0, pos + 4);
			return str;
		};

		auto vecToString = [=](auto vec) -> std::string {
			return floatToString(vec.x) + ", "
				+ floatToString(vec.y) + ", "
				+ floatToString(vec.z);
		};

		uint32_t triangleCount = 0;
		auto sceneAABB = meshes.front().data.aabb;
		for (auto& mesh : meshes) {
			sceneAABB.Grow(mesh.data.aabb);
			triangleCount += mesh.data.GetIndexCount() / 3;
		}

		if (ImGui::Begin("Settings", (bool*)0, 0)) {
			ImGui::Text(("Average frametime: " + std::to_string(averageFramerate * 1000.0f) + " ms").c_str());
			ImGui::Text(("Current frametime: " + std::to_string(deltaTime * 1000.0f) + " ms").c_str());
			ImGui::Text(("Camera location: " + vecToString(camera.location)).c_str());
			ImGui::Text(("Scene dimensions: " + vecToString(sceneAABB.min) + " to " + vecToString(sceneAABB.max)).c_str());
			ImGui::Text(("Scene triangle count: " + std::to_string(triangleCount)).c_str());

			{
				const char* items[] = { "Cornell box", "Sponza", "Bistro", 
					"San Miguel", "Medieval", "Pica Pica", "New Sponza"};
				int currentItem = static_cast<int>(sceneSelection);
				ImGui::Combo("Select scene", &currentItem, items, IM_ARRAYSIZE(items));

				if (currentItem != sceneSelection) {
					auto newSceneSelection = static_cast<SceneSelection>(currentItem);
					if (IsSceneAvailable(newSceneSelection)) {
						sceneSelection = newSceneSelection;
						UnloadScene();
						LoadScene();
					}
					else {
						openSceneNotFoundPopup = true;
					}
				}
			}

			ImGui::Checkbox("Pathtrace", &pathTrace);

			if (pathTrace) ImGui::SliderInt("Pathtrace bounces", &pathTracingRenderer.bounces, 0, 100);

			if (ImGui::CollapsingHeader("General")) {
				static bool fullscreenMode = false;
				static bool vsyncMode = false;

				bool fullscreen = fullscreenMode;
				bool vsync = vsyncMode;

				ImGui::Checkbox("VSync", &vsync);
				ImGui::Checkbox("Fullscreen", &fullscreen);

				if (vsync != vsyncMode) {
					if (vsync) LockFramerate();
					else UnlockFramerate();
					vsyncMode = vsync;
				}
				if (fullscreen != fullscreenMode) {
					if (fullscreen) {
						windowWidth = window.GetWidth();
						windowHeight = window.GetHeight();
						window.SetSize(GetScreenSize().x, GetScreenSize().y);
						window.SetFullscreen(true);
					}
					else {
						window.SetSize(windowWidth, windowHeight);
						window.SetFullscreen(false);
					}
					fullscreenMode = fullscreen;
				}

				const char* items[] = { "1280x720", "1920x1080", "2560x1440", "3840x2160" };
				static int resolution = 1;
				int currentItem = resolution;
				ImGui::Combo("Resolution##Rendering", &currentItem, items, IM_ARRAYSIZE(items));

				if (currentItem != resolution) {
					resolution = currentItem;
					switch (resolution) {
					case 0: SetResolution(1280, 720); break;
					case 1: SetResolution(1920, 1080); break;
					case 2: SetResolution(2560, 1440); break;
					case 3: SetResolution(3840, 2160); break;
					}
				}

			}

			if (ImGui::CollapsingHeader("DDGI")) {
				ImGui::Text(("Probe count: " + vecToString(volume->probeCount)).c_str());
				ImGui::Text(("Cell size: " + vecToString(volume->cellSize)).c_str());
				ImGui::Checkbox("Enable volume", &volume->enable);
				ImGui::Checkbox("Update volume", &volume->update);
				ImGui::Checkbox("Visualize probes", &volume->debug);
				ImGui::Checkbox("Sample emissives", &volume->sampleEmissives);

				const char* items[] = { "5x5x5", "10x10x10", "20x20x20", "30x30x30" };
				int currentItem = 0;
				if (volume->probeCount == ivec3(5)) currentItem = 0;
				if (volume->probeCount == ivec3(10)) currentItem = 1;
				if (volume->probeCount == ivec3(20)) currentItem = 2;
				if (volume->probeCount == ivec3(30)) currentItem = 3;
				auto prevItem = currentItem;
				ImGui::Combo("Resolution##DDGI", &currentItem, items, IM_ARRAYSIZE(items));

				if (currentItem != prevItem) {
					switch (currentItem) {
					case 0: volume->SetProbeCount(ivec3(5)); break;
					case 1: volume->SetProbeCount(ivec3(10)); break;
					case 2: volume->SetProbeCount(ivec3(20)); break;
					case 3: volume->SetProbeCount(ivec3(30)); break;
					}
				}

				ImGui::SliderFloat("Strength##DDGI", &volume->strength, 0.0f, 5.0f);
				ImGui::Separator();
				ImGui::Text("AABB");
				ImGui::SliderFloat3("Min", (float*)&volume->aabb.min, -200.0f, 200.0f);
				ImGui::SliderFloat3("Max", (float*)&volume->aabb.max, -200.0f, 200.0f);
				volume->SetAABB(volume->aabb);
				ImGui::Separator();
				ImGui::SliderFloat("Hysteresis", &volume->hysteresis, 0.0f, 1.0f, "%.3f", 0.5f);
				ImGui::SliderFloat("Sharpness", &volume->sharpness, 0.01f, 200.0f, "%.3f", 2.0f);
				ImGui::SliderFloat("Bias", &volume->bias, 0.0f, 1.0f);
				auto prevGamma = volume->gamma;
				ImGui::SliderFloat("Gamma exponent", &volume->gamma, 0.0f, 10.0f, "%.3f", 2.0f);
				if (prevGamma != volume->gamma) volume->ClearProbes();
				ImGui::Separator();
				if (ImGui::Button("Reset probe offsets")) {
					volume->ResetProbeOffsets();
				}
				ImGui::Checkbox("Optimize probes", &volume->optimizeProbes);
			}
			if (ImGui::CollapsingHeader("Light")) {
				ImGui::Checkbox("Animate", &animateLight);
				ImGui::SliderFloat3("Direction", (float*)&light.direction, -1.0f, 1.0f);
				ImGui::ColorEdit3("Color", (float*)&light.color);
				ImGui::SliderFloat("Intensity##Light", &light.intensity, 0.0, 1000.0f, "%.3f", 2.0f);
				ImGui::Separator();
				ImGui::Text("Volumetric");
				ImGui::SliderFloat("Intensity##Volumetric", &light.GetVolumetric()->intensity, 0.0f, 1.0f);
				ImGui::Text("Shadow");
				ImGui::SliderFloat("Bias##Shadow", &light.GetShadow()->bias, 0.0f, 2.0f);
			}
			if (ImGui::CollapsingHeader("Ambient Occlusion")) {
				ImGui::Checkbox("Debug", &showAo);
				ImGui::Checkbox("Enable ambient occlusion", &ssao->enable);
				ImGui::SliderFloat("Radius", &ssao->radius, 0.0f, 10.0f);
				ImGui::SliderFloat("Strength", &ssao->strength, 0.0f, 20.0f, "%.3f", 2.0f);
			}
			if (ImGui::CollapsingHeader("Camera")) {
				ImGui::SliderFloat("Exposure##Camera", &camera.exposure, 0.0f, 10.0f);
				ImGui::SliderFloat("Speed##Camera", &cameraSpeed, 0.0f, 20.0f);
				ImGui::SliderFloat("FOV##Camera", &camera.fieldOfView, 0.0f, 90.0f);
				keyboardHandler.speed = cameraSpeed;
			}
			if (ImGui::CollapsingHeader("Fog")) {
				ImGui::Checkbox("Enable##Fog", &fog->enable);
				fog->color = glm::pow(fog->color, 1.0f / vec3(2.2f));
				ImGui::ColorEdit3("Color##Fog", &fog->color[0]);
				fog->color = glm::pow(fog->color, vec3(2.2f));

				ImGui::SliderFloat("Density##Fog", &fog->density, 0.0f, 0.5f, "%.4f", 4.0f);
				ImGui::SliderFloat("Height##Fog", &fog->height, 0.0f, 300.0f, "%.3f", 4.0f);
				ImGui::SliderFloat("Height falloff##Fog", &fog->heightFalloff, 0.0f, 0.5f, "%.4f", 4.0f);
				ImGui::SliderFloat("Scattering anisotropy##Fog", &fog->scatteringAnisotropy, -1.0f, 1.0f, "%.3f", 2.0f);
			}
			if (ImGui::CollapsingHeader("Postprocessing")) {
				ImGui::Text("Temporal anti-aliasing");
				ImGui::Checkbox("Enable##TAA", &scene.postProcessing.taa.enable);
				ImGui::Checkbox("Enable slow mode##SlowMode", &slowMode);
				ImGui::SliderFloat("Jitter range##TAA", &scene.postProcessing.taa.jitterRange, 0.001f, 0.999f);
				ImGui::Separator();
				ImGui::Text("Sharpen filter");
				ImGui::Checkbox("Enable##Sharpen", &scene.postProcessing.sharpen.enable);
				ImGui::SliderFloat("Sharpness", &scene.postProcessing.sharpen.factor, 0.0f, 1.0f);
			}
			if (ImGui::CollapsingHeader("Controls")) {
				ImGui::Text("Use WASD for movement");
				ImGui::Text("Use left mouse click + mouse movement to look around");
				ImGui::Text("Use F11 to hide/unhide the UI");
			}
			if (ImGui::CollapsingHeader("Profiler")) {

				const char* items[] = { "Chronologically", "Max time", "Min time" };
				static int item = 0;
				ImGui::Combo("Sort##Performance", &item, items, IM_ARRAYSIZE(items));

				Atlas::Profiler::OrderBy order;
				switch (item) {
				case 1: order = Atlas::Profiler::OrderBy::MAX_TIME; break;
				case 2: order = Atlas::Profiler::OrderBy::MIN_TIME; break;
				default: order = Atlas::Profiler::OrderBy::CHRONO; break;
				}

				std::function<void(Atlas::Profiler::Query&)> displayQuery;
				displayQuery = [&displayQuery](Atlas::Profiler::Query& query) {
					ImGui::TableNextRow();
					ImGui::TableNextColumn();

					ImGuiTreeNodeFlags expandable = 0;
					if (!query.children.size()) expandable = ImGuiTreeNodeFlags_NoTreePushOnOpen |
						ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

					bool open = ImGui::TreeNodeEx(query.name.c_str(), expandable | ImGuiTreeNodeFlags_SpanFullWidth);
					ImGui::TableNextColumn();
					ImGui::Text("%f", double(query.timer.elapsedTime) / 1000000.0);
					// ImGui::TableNextColumn();
					// ImGui::TextUnformatted(node->Type);

					if (open && query.children.size()) {
						for (auto& child : query.children)
							displayQuery(child);
						ImGui::TreePop();
					}

				};

				static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;

				if (ImGui::BeginTable("PerfTable", 2, flags))
				{
					// The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
					ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
					ImGui::TableSetupColumn("Elapsed (ms)", ImGuiTableColumnFlags_NoHide);
					//ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_NoHide);
					ImGui::TableHeadersRow();

					auto queries = Atlas::Profiler::GetQueriesAverage(64, order);
					for (auto& query : queries)
						displayQuery(query);

					ImGui::EndTable();
				}
			}

			ImGui::End();
		}

		if (openSceneNotFoundPopup) {
			ImGui::OpenPopup("Scene not found");
		}

		if (ImGui::BeginPopupModal("Scene not found")) {
			ImGui::Text("Please download additional scenes with the download script in the data directory");
			ImGui::Text("There is a script for both Linux and Windows");
			ImGui::Text("Note: Not all scene might be downloadable");
			if (ImGui::Button("Close##SceneNotFound")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::Render();
		imguiWrapper.Render();
	}

	if (slowMode) { using namespace std::chrono_literals; std::this_thread::sleep_for(60ms); }

	if (firstFrame) {
		// We want to get rid of the current average
		// window which includes the loading times
		Atlas::Clock::ResetAverage();
		firstFrame = false;
	}


}

void App::DisplayLoadingScreen() {

	float textWidth, textHeight;
	font.ComputeDimensions("Loading...", 2.5f, &textWidth, &textHeight);

	window.Clear();

	auto windowSize = window.GetDrawableSize();

	float x = windowSize.x / 2 - textWidth / 2;
	float y = windowSize.y / 2 - textHeight / 2;

	viewport.Set(0, 0, windowSize.x, windowSize.y);
	masterRenderer.textRenderer.Render(&viewport, &font, "Loading...", x, y, vec4(1.0f, 1.0f, 1.0f, 1.0f), 2.5f);

	window.Update();

}

bool App::IsSceneAvailable(SceneSelection selection) {
	switch (selection) {
	case CORNELL: return Atlas::Loader::AssetLoader::FileExists("cornell/CornellBox-Original.obj");
	case SPONZA: return Atlas::Loader::AssetLoader::FileExists("sponza/sponza.obj");
	case BISTRO: return Atlas::Loader::AssetLoader::FileExists("bistro/mesh/exterior.obj");
	case SANMIGUEL: return Atlas::Loader::AssetLoader::FileExists("sanmiguel/san-miguel-low-poly.obj");
	case MEDIEVAL: return Atlas::Loader::AssetLoader::FileExists("medieval/scene.fbx");
	case PICAPICA: return Atlas::Loader::AssetLoader::FileExists("pica pica/mesh/scene.gltf");
	case NEWSPONZA: return Atlas::Loader::AssetLoader::FileExists("newsponza/NewSponza_Main_Blender_glTF.gltf");
	default: return false;
	}
}

bool App::LoadScene() {

	bool successful = false;

	DisplayLoadingScreen();

	Atlas::Texture::Cubemap sky;
	directionalLight.direction = vec3(0.0f, -1.0f, 1.0f);

	if (sceneSelection == CORNELL) {
		meshes.reserve(1);

		auto meshData = Atlas::Loader::ModelLoader::LoadMesh("cornell/CornellBox-Original.obj");
		meshes.push_back(Atlas::Mesh::Mesh { meshData });

		auto& mesh = meshes.back();
		mesh.invertUVs = true;
		mesh.SetTransform(scale(mat4(1.0f), vec3(10.0f)));
		scene.irradianceVolume = new Atlas::Lighting::IrradianceVolume(mesh.data.aabb.Scale(1.10f), ivec3(20));
		scene.irradianceVolume->sampleEmissives = true;

		// Other scene related settings apart from the mesh
		directionalLight.intensity = 0.0f;
		directionalLight.GetVolumetric()->intensity = 0.0f;
		scene.irradianceVolume->SetRayCount(512, 32);

		// Setup camera
		camera.location = vec3(0.0f, 14.0f, 40.0f);
		camera.rotation = vec2(-3.14f, -0.1f);

		scene.fog->enable = false;
	}
	else if (sceneSelection == SPONZA) {
		meshes.reserve(1);

		auto meshData = Atlas::Loader::ModelLoader::LoadMesh("sponza/sponza.obj");
		meshes.push_back(Atlas::Mesh::Mesh{ meshData });

		auto& mesh = meshes.back();
		mesh.invertUVs = true;
		mesh.SetTransform(scale(mat4(1.0f), vec3(.05f)));
		scene.irradianceVolume = new Atlas::Lighting::IrradianceVolume(mesh.data.aabb.Scale(0.90f), ivec3(20));

		sky = Atlas::Texture::Cubemap("environment.hdr", 1024);

		// Other scene related settings apart from the mesh
		directionalLight.intensity = 100.0f;
		directionalLight.GetVolumetric()->intensity = 0.28f;
		scene.irradianceVolume->SetRayCount(128, 32);

		// Setup camera
		camera.location = vec3(30.0f, 25.0f, 0.0f);
		camera.rotation = vec2(-3.14f / 2.0f, 0.0f);

		scene.fog->enable = true;
	}
	else if (sceneSelection == BISTRO) {
		meshes.reserve(1);

		auto meshData = Atlas::Loader::ModelLoader::LoadMesh("bistro/mesh/exterior.obj", false, mat4(1.0f), 2048);
		meshes.push_back(Atlas::Mesh::Mesh{ meshData });

		auto& mesh = meshes.back();
		mesh.invertUVs = true;
		mesh.SetTransform(scale(mat4(1.0f), vec3(.015f)));
		scene.irradianceVolume = new Atlas::Lighting::IrradianceVolume(mesh.data.aabb.Scale(0.90f), ivec3(20));

		sky = Atlas::Texture::Cubemap("environment.hdr", 1024);

		// Other scene related settings apart from the mesh
		directionalLight.intensity = 100.0f;
		directionalLight.GetVolumetric()->intensity = 0.28f;
		scene.irradianceVolume->SetRayCount(32, 32);

		// Setup camera
		camera.location = vec3(-21.0f, 8.0f, 1.0f);
		camera.rotation = vec2(3.14f / 2.0f, 0.0f);

		scene.fog->enable = true;
	}
	else if (sceneSelection == SANMIGUEL) {
		meshes.reserve(1);

		auto meshData = Atlas::Loader::ModelLoader::LoadMesh("sanmiguel/san-miguel-low-poly.obj", false, mat4(1.0f), 2048);
		meshes.push_back(Atlas::Mesh::Mesh{ meshData });

		auto& mesh = meshes.back();
		mesh.invertUVs = true;
		mesh.cullBackFaces = false;
		mesh.SetTransform(scale(mat4(1.0f), vec3(2.0f)));
		scene.irradianceVolume = new Atlas::Lighting::IrradianceVolume(mesh.data.aabb.Scale(1.0f), ivec3(20));

		sky = Atlas::Texture::Cubemap("environment.hdr", 1024);

		// Other scene related settings apart from the mesh
		directionalLight.intensity = 100.0f;
		directionalLight.GetVolumetric()->intensity = 0.28f;
		directionalLight.direction = vec3(0.0f, -1.0f, -1.0f);
		scene.irradianceVolume->SetRayCount(128, 32);

		// Setup camera
		camera.location = vec3(45.0f, 26.0f, 17.0f);
		camera.rotation = vec2(-4.14f / 2.0f, -.6f);
		camera.exposure = 2.5f;

		scene.fog->enable = true;
	}
	else if (sceneSelection == MEDIEVAL) {
		meshes.reserve(1);

		auto meshData = Atlas::Loader::ModelLoader::LoadMesh("medieval/scene.fbx");
		meshes.push_back(Atlas::Mesh::Mesh{ meshData });

		auto& mesh = meshes.back();
		mesh.invertUVs = true;
		mesh.SetTransform(scale(glm::rotate(glm::mat4(1.0f), -3.14f / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f)), vec3(2.0f)));
		// Metalness is set to 0.9f
		for (auto& material : mesh.data.materials) material.metalness = 0.0f;

		scene.irradianceVolume = new Atlas::Lighting::IrradianceVolume(mesh.data.aabb.Scale(1.0f), ivec3(20));

		sky = Atlas::Texture::Cubemap("environment.hdr", 1024);

		// Other scene related settings apart from the mesh
		directionalLight.intensity = 10.0f;
		directionalLight.GetVolumetric()->intensity = 0.08f;
		scene.irradianceVolume->SetRayCount(128, 32);

		// Setup camera
		camera.location = vec3(30.0f, 25.0f, 0.0f);
		camera.rotation = vec2(-3.14f / 2.0f, 0.0f);

		scene.fog->enable = true;
	}
	else if (sceneSelection == PICAPICA) {
		meshes.reserve(1);

		auto meshData = Atlas::Loader::ModelLoader::LoadMesh("pica pica/mesh/scene.gltf");
		meshes.push_back(Atlas::Mesh::Mesh{ meshData });

		auto& mesh = meshes.back();
		mesh.invertUVs = true;

		scene.irradianceVolume = new Atlas::Lighting::IrradianceVolume(mesh.data.aabb.Scale(1.0f), ivec3(20));

		sky = Atlas::Texture::Cubemap("environment.hdr", 1024);

		// Other scene related settings apart from the mesh
		directionalLight.intensity = 10.0f;
		directionalLight.GetVolumetric()->intensity = 0.08f;
		scene.irradianceVolume->SetRayCount(128, 32);

		// Setup camera
		camera.location = vec3(30.0f, 25.0f, 0.0f);
		camera.rotation = vec2(-3.14f / 2.0f, 0.0f);

		scene.fog->enable = true;
	}
	else if (sceneSelection == NEWSPONZA) {
		meshes.reserve(3);

		auto meshData = Atlas::Loader::ModelLoader::LoadMesh("newsponza/NewSponza_Main_Blender_glTF.gltf", 
			false, glm::scale(mat4(1.0f), vec3(4.0f)), 2048);
		meshes.push_back(Atlas::Mesh::Mesh{ meshData });
		meshes.back().invertUVs = true;

		meshData = Atlas::Loader::ModelLoader::LoadMesh("newsponza/NewSponza_100sOfCandles_glTF_OmniLights.gltf", 
			false, glm::scale(mat4(1.0f), vec3(4.0f)), 2048);
		meshes.push_back(Atlas::Mesh::Mesh{ meshData });
		meshes.back().invertUVs = true;

		meshData = Atlas::Loader::ModelLoader::LoadMesh("newsponza/NewSponza_Curtains_glTF.gltf", 
			false, glm::scale(mat4(1.0f), vec3(4.0f)), 2048);
		meshes.push_back(Atlas::Mesh::Mesh{ meshData });
		meshes.back().invertUVs = true;
		meshes.back().cullBackFaces = false;

		scene.irradianceVolume = new Atlas::Lighting::IrradianceVolume(meshes.front().data.aabb.Scale(1.05f), ivec3(20));

		sky = Atlas::Texture::Cubemap("environment.hdr", 1024);

		// Other scene related settings apart from the mesh
		directionalLight.intensity = 100.0f;
		directionalLight.GetVolumetric()->intensity = 0.28f;
		scene.irradianceVolume->SetRayCount(128, 32);

		// Setup camera
		camera.location = vec3(30.0f, 25.0f, 0.0f);
		camera.rotation = vec2(-3.14f / 2.0f, 0.0f);

		scene.fog->enable = true;
	}

	actors.reserve(meshes.size());
	for (auto& mesh : meshes) {
		actors.push_back({ &mesh, mat4(1.0f) });
		scene.Add(&actors.back());
	}

	scene.sky.probe = new Atlas::Lighting::EnvironmentProbe(sky);

	camera.Update();
	scene.Update(&camera, 1.0f);
	scene.BuildRTStructures();

	// Reset input handlers
	keyboardHandler.Reset(&camera);
	mouseHandler.Reset(&camera);

	Atlas::Clock::ResetAverage();

	return successful;

}

void App::UnloadScene() {

	for (auto& actor : actors) scene.Remove(&actor);

	actors.clear();
	meshes.clear();

	delete scene.sky.probe;
	delete scene.irradianceVolume;

}

void App::SetResolution(int32_t width, int32_t height) {

	renderTarget->Resize(width, height);
	pathTraceTarget.Resize(width, height);

}

Atlas::EngineInstance* GetEngineInstance() {

	return new App();

}
