#include "App.h"

#include <chrono>
#include <thread>

const Atlas::EngineConfig Atlas::EngineInstance::engineConfig = {
    .assetDirectory = "../../data",
    .shaderDirectory = "shader"
};

void App::LoadContent() {

    renderTarget = Atlas::RenderTarget(1920, 1080);

    auto icon = Atlas::Texture::Texture2D("icon.png");
    window.SetIcon(&icon);

    loadingTexture = Atlas::CreateRef<Atlas::Texture::Texture2D>("loading.png");

    font = Atlas::Font("font/roboto.ttf", 22, 5);

    camera = Atlas::Camera(47.0f, 2.0f, 1.0f, 400.0f,
        glm::vec3(30.0f, 25.0f, 0.0f), glm::vec2(-3.14f / 2.0f, 0.0f));

    scene = Atlas::CreateRef<Atlas::Scene::Scene>(glm::vec3(-2048.0f), glm::vec3(2048.0f));

    mouseHandler = Atlas::Input::MouseHandler(&camera, 1.5f, 6.0f);
    keyboardHandler = Atlas::Input::KeyboardHandler(&camera, 7.0f, 6.0f);
    controllerHandler = Atlas::Input::ControllerHandler(&camera, 1.5f, 5.0f, 10.0f, 5000.0f);

    Atlas::Events::EventManager::KeyboardEventDelegate.Subscribe(
        [this](Atlas::Events::KeyboardEvent event) {
            if (event.keyCode == AE_KEY_ESCAPE) {
                Exit();
            }
            if (event.keyCode == AE_KEY_F11 && event.state == AE_BUTTON_RELEASED) {
                renderUI = !renderUI;
            }
            if (event.keyCode == AE_KEY_LSHIFT && event.state == AE_BUTTON_PRESSED) {
                keyboardHandler.speed = cameraSpeed * 4.0f;
            }
            if (event.keyCode == AE_KEY_LSHIFT && event.state == AE_BUTTON_RELEASED) {
                keyboardHandler.speed = cameraSpeed;
            }
        });
    
    Atlas::PipelineManager::EnableHotReload();

    directionalLight = Atlas::CreateRef<Atlas::Lighting::DirectionalLight>(AE_MOVABLE_LIGHT);
    directionalLight->direction = glm::vec3(0.0f, -1.0f, 1.0f);
    directionalLight->color = glm::vec3(255, 236, 209) / 255.0f;
    glm::mat4 orthoProjection = glm::ortho(-100.0f, 100.0f, -70.0f, 120.0f, -120.0f, 120.0f);
    directionalLight->AddShadow(200.0f, 3.0f, 4096, glm::vec3(0.0f), orthoProjection);
    directionalLight->AddVolumetric(10, 0.28f);

    scene->sky.sun = directionalLight;

    scene->sky.atmosphere = Atlas::CreateRef<Atlas::Lighting::Atmosphere>();

    scene->postProcessing.taa = Atlas::PostProcessing::TAA(0.99f);
    scene->postProcessing.sharpen.enable = true;
    scene->postProcessing.sharpen.factor = 0.15f;

    LoadScene();

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    imguiWrapper.Load(&window);

}

void App::UnloadContent() {

    UnloadScene();
    imguiWrapper.Unload();

}

void App::Update(float deltaTime) {

    if (sceneReload) {
        UnloadScene();
        LoadScene();
        sceneReload = false;
    }

    const ImGuiIO& io = ImGui::GetIO();

    imguiWrapper.Update(&window, deltaTime);

    if (io.WantCaptureMouse) {
        mouseHandler.lock = true;
    }
    else {
        mouseHandler.lock = false;
    }

    if (controllerHandler.IsControllerAvailable()) {
        controllerHandler.Update(&camera, deltaTime);
    }
    else {
        mouseHandler.Update(&camera, deltaTime);
        keyboardHandler.Update(&camera, deltaTime);
    }    

    if (rotateCamera) {
        camera.rotation.y += rotateCameraSpeed * cos(Atlas::Clock::Get());
        mouseHandler.Reset(&camera);
    }

    if(moveCamera) {
        camera.location += camera.right * moveCameraSpeed * cos(Atlas::Clock::Get());
        mouseHandler.Reset(&camera);
    }

    camera.UpdateView();
    camera.UpdateProjection();

    if (sceneSelection == SPONZA) {
        auto matrix = glm::translate(glm::mat4(1.0f), glm::vec3(-50.0f, 0.0f, -2.0f));
        matrix = glm::rotate(matrix, Atlas::Clock::Get() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));

        actors[1].SetMatrix(matrix);

        float height = (sinf(Atlas::Clock::Get() / 5.0f) + 1.0f) * 20.0f;
        matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, height, .0f));

        actors[2].SetMatrix(matrix);
    }

    scene->Update(&camera, deltaTime);

    CheckLoadScene();

}

void App::Render(float deltaTime) {

    static bool firstFrame = true;
    static bool animateLight = false;
    static bool pathTrace = false;
    static bool debugAo = false;
    static bool debugReflection = false;
    static bool debugClouds = false;
    static bool debugSSS = false;
    static bool debugSSGI = false;
    static bool debugMotion = false;
    static bool slowMode = false;

    static float cloudDepthDebug = 0.0f;

#ifndef AE_HEADLESS
    auto windowFlags = window.GetFlags();
    if (windowFlags & AE_WINDOW_HIDDEN || windowFlags & AE_WINDOW_MINIMIZED || !(windowFlags & AE_WINDOW_SHOWN)) {
        return;
    }
#endif

    if (!loadingComplete) {
        DisplayLoadingScreen(deltaTime);
        return;
    }

    if (animateLight) directionalLight->direction = glm::vec3(0.0f, -1.0f, sin(Atlas::Clock::Get() / 10.0f));


    {
        mainRenderer->RenderScene(&viewport, &renderTarget, &camera, scene.get());

        auto debug = debugAo || debugReflection || debugClouds || debugSSS || debugSSGI || debugMotion;

        if (debug && graphicsDevice->swapChain->isComplete) {
            auto commandList = graphicsDevice->GetCommandList(Atlas::Graphics::GraphicsQueue);
            commandList->BeginCommands();
            commandList->BeginRenderPass(graphicsDevice->swapChain, true);

            if (debugAo) {
                mainRenderer->textureRenderer.RenderTexture2D(commandList, &viewport, &renderTarget.aoTexture,
                    0.0f, 0.0f, float(viewport.width), float(viewport.height), 0.0, 1.0f, false, true);
            }
            else if (debugReflection) {
                mainRenderer->textureRenderer.RenderTexture2D(commandList, &viewport, &renderTarget.reflectionTexture,
                    0.0f, 0.0f, float(viewport.width), float(viewport.height), 0.0, 1.0f, false, true);
            }
            else if (debugClouds) {
                mainRenderer->textureRenderer.RenderTexture2D(commandList, &viewport, &renderTarget.volumetricCloudsTexture,
                    0.0f, 0.0f, float(viewport.width), float(viewport.height), 0.0, 1.0f, false, true);
            }
            else if (debugSSS) {
                mainRenderer->textureRenderer.RenderTexture2D(commandList, &viewport, &renderTarget.sssTexture,
                    0.0f, 0.0f, float(viewport.width), float(viewport.height), 0.0, 1.0f, false, true);
            }
            else if (debugSSGI) {
                mainRenderer->textureRenderer.RenderTexture2D(commandList, &viewport, &renderTarget.giTexture,
                    0.0f, 0.0f, float(viewport.width), float(viewport.height), 0.0, 1.0f, false, true);
            }
            else if (debugMotion) {
                mainRenderer->textureRenderer.RenderTexture2D(commandList, &viewport,
                    renderTarget.GetData(Atlas::FULL_RES)->velocityTexture.get(),
                    0.0f, 0.0f, float(viewport.width), float(viewport.height), 0.0, 10.0f, false, true);
            }

            commandList->EndRenderPass();
            commandList->EndCommands();
            graphicsDevice->SubmitCommandList(commandList);
        }
    }

    float averageFramerate = Atlas::Clock::GetAverage();

    // ImGui rendering
    if (renderUI) {
        static bool recreateSwapchain = false;

        ImGui::NewFrame();

        const auto& light = directionalLight;
        const auto& clouds = scene->sky.clouds;
        auto& postProcessing = scene->postProcessing;

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
        auto sceneAABB = Atlas::Volume::AABB();
        for (auto& mesh : meshes) {
            if (!mesh.IsLoaded())
                continue;
            sceneAABB.Grow(mesh->data.aabb);
            triangleCount += mesh->data.GetIndexCount() / 3;
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

#ifdef AE_HEADLESS
        Atlas::Log::Message("Frame rendererd");
        renderTarget.hdrTexture.Save<float>("prepost");
        renderTarget.postProcessTexture.Save<uint8_t>("result");
#endif

        if (!recreateSwapchain) {
            imguiWrapper.Render();
        }

        recreateSwapchain = false;
    }

    if (slowMode) { using namespace std::chrono_literals; std::this_thread::sleep_for(60ms); }

    if (firstFrame) {
        // We want to get rid of the current average
        // window which includes the loading times
        Atlas::Clock::ResetAverage();
        firstFrame = false;
    }

}

void App::DisplayLoadingScreen(float deltaTime) {

    auto commandList = graphicsDevice->GetCommandList();

    commandList->BeginCommands();
    graphicsDevice->swapChain->colorClearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};
    commandList->BeginRenderPass(graphicsDevice->swapChain, true);

    auto windowSize = window.GetDrawableSize();

    float width = float(loadingTexture->width);
    float height = float(loadingTexture->height);

    float x = windowSize.x / 2 - width / 2;
    float y = windowSize.y / 2 - height / 2;

    static float rotation = 0.0f;

    rotation += deltaTime * abs(sin(Atlas::Clock::Get())) * 10.0f;

    mainRenderer->textureRenderer.RenderTexture2D(commandList, &viewport,
        loadingTexture.get(), x, y, width, height, rotation);

    float textWidth, textHeight;
    font.ComputeDimensions("Loading...", 2.0f, &textWidth, &textHeight);

    x = windowSize.x / 2 - textWidth / 2;
    y = windowSize.y / 2 - textHeight / 2 + float(loadingTexture->height) + 20.0f;

    viewport.Set(0, 0, windowSize.x, windowSize.y);
    mainRenderer->textRenderer.Render(commandList, &viewport, &font,
        "Loading...", x, y, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 2.0f);

    commandList->EndRenderPass();
    commandList->EndCommands();

    graphicsDevice->SubmitCommandList(commandList);

}

bool App::IsSceneAvailable(SceneSelection selection) {
    switch (selection) {
        case CORNELL: return Atlas::Loader::AssetLoader::FileExists("cornell/CornellBox-Original.obj");
        case SPONZA: return Atlas::Loader::AssetLoader::FileExists("sponza/sponza.obj");
        case BISTRO: return Atlas::Loader::AssetLoader::FileExists("bistro/mesh/exterior.obj");
        case SANMIGUEL: return Atlas::Loader::AssetLoader::FileExists("sanmiguel/san-miguel-low-poly.obj");
        case MEDIEVAL: return Atlas::Loader::AssetLoader::FileExists("medieval/scene.fbx");
        case PICAPICA: return Atlas::Loader::AssetLoader::FileExists("pica pica/mesh/scene.gltf");
        case SUBWAY: return Atlas::Loader::AssetLoader::FileExists("subway/scene.gltf");
        case MATERIALS: return Atlas::Loader::AssetLoader::FileExists("material demo/materials.obj");
        case FOREST: return Atlas::Loader::AssetLoader::FileExists("forest/forest.gltf");
        case EMERALDSQUARE: return Atlas::Loader::AssetLoader::FileExists("emeraldsquare/square.gltf");
        case FLYINGWORLD: return Atlas::Loader::AssetLoader::FileExists("flying world/scene.gltf");
        case NEWSPONZA: return Atlas::Loader::AssetLoader::FileExists("newsponza/main/NewSponza_Main_Blender_glTF.gltf") &&
                               Atlas::Loader::AssetLoader::FileExists("newsponza/candles/NewSponza_100sOfCandles_glTF_OmniLights.gltf") &&
                               Atlas::Loader::AssetLoader::FileExists("newsponza/curtains/NewSponza_Curtains_glTF.gltf") &&
                               Atlas::Loader::AssetLoader::FileExists("newsponza/ivy/NewSponza_IvyGrowth_glTF.gltf");
        default: return false;
    }
}

bool App::LoadScene() {

    bool successful = false;
    loadingComplete = false;

    Atlas::Texture::Cubemap sky;
    directionalLight->direction = glm::vec3(0.0f, -1.0f, 1.0f);

    scene->sky.clouds = Atlas::CreateRef<Atlas::Lighting::VolumetricClouds>();
    scene->sky.clouds->minHeight = 1400.0f;
    scene->sky.clouds->maxHeight = 1700.0f;
    scene->sky.clouds->castShadow = false;

    scene->sky.probe = nullptr;
    scene->sky.clouds->enable = true;

    using namespace Atlas::Loader;

    if (sceneSelection == CORNELL) {
        meshes.reserve(1);

        glm::mat4 transform = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f));
        auto mesh = Atlas::ResourceManager<Atlas::Mesh::Mesh>::GetOrLoadResourceWithLoaderAsync(
            "cornell/CornellBox-Original.obj", ModelLoader::LoadMesh, false, transform, 2048
        );
        meshes.push_back(mesh);

        // Other scene related settings apart from the mesh
        directionalLight->intensity = 0.0f;
        directionalLight->GetVolumetric()->intensity = 0.0f;

        // Setup camera
        camera.location = glm::vec3(0.0f, 14.0f, 40.0f);
        camera.rotation = glm::vec2(-3.14f, -0.1f);

    }
    else if (sceneSelection == SPONZA) {
        meshes.reserve(1);

        glm::mat4 transform = glm::scale(glm::mat4(1.0f), glm::vec3(.05f));
        auto mesh = Atlas::ResourceManager<Atlas::Mesh::Mesh>::GetOrLoadResourceWithLoaderAsync(
            "sponza/sponza.obj", ModelLoader::LoadMesh, false, transform, 2048
        );
        meshes.push_back(mesh);
 
        transform = glm::scale(glm::mat4(1.0f), glm::vec3(1.f));
        mesh = Atlas::ResourceManager<Atlas::Mesh::Mesh>::GetOrLoadResourceWithLoaderAsync(
            "metallicwall.gltf", ModelLoader::LoadMesh, Atlas::Mesh::MeshMobility::Movable,
            false, transform, 2048
        );
        meshes.push_back(mesh);

        transform = glm::scale(glm::mat4(1.0f), glm::vec3(1.f));
        mesh = Atlas::ResourceManager<Atlas::Mesh::Mesh>::GetOrLoadResourceWithLoaderAsync(
            "chromesphere.gltf", ModelLoader::LoadMesh, Atlas::Mesh::MeshMobility::Movable,
            false, transform, 2048
        );
        meshes.push_back(mesh);
       

        // Other scene related settings apart from the mesh
        directionalLight->direction = glm::vec3(0.0f, -1.0f, 0.33f);
        directionalLight->intensity = 100.0f;
        directionalLight->GetVolumetric()->intensity = 0.28f;

        // Setup camera
        camera.location = glm::vec3(30.0f, 25.0f, 0.0f);
        camera.rotation = glm::vec2(-3.14f / 2.0f, 0.0f);
        camera.exposure = 0.125f;

    }
    else if (sceneSelection == BISTRO) {
        meshes.reserve(1);

        auto transform = glm::scale(glm::mat4(1.0f), glm::vec3(.015f));
        auto mesh = Atlas::ResourceManager<Atlas::Mesh::Mesh>::GetOrLoadResourceWithLoaderAsync(
            "bistro/mesh/exterior.obj", ModelLoader::LoadMesh, false, transform, 2048
        );
        meshes.push_back(mesh);

        // Other scene related settings apart from the mesh
        directionalLight->intensity = 100.0f;
        directionalLight->GetVolumetric()->intensity = 0.28f;

        // Setup camera
        camera.location = glm::vec3(-21.0f, 8.0f, 1.0f);
        camera.rotation = glm::vec2(3.14f / 2.0f, 0.0f);
        camera.exposure = 0.125f;

    }
    else if (sceneSelection == SANMIGUEL) {
        meshes.reserve(1);

        auto transform = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));
        auto mesh = Atlas::ResourceManager<Atlas::Mesh::Mesh>::GetOrLoadResourceWithLoaderAsync(
            "sanmiguel/san-miguel-low-poly.obj", ModelLoader::LoadMesh, false, transform, 2048
        );
        meshes.push_back(mesh);

        // Other scene related settings apart from the mesh
        directionalLight->intensity = 100.0f;
        directionalLight->GetVolumetric()->intensity = 0.28f;
        directionalLight->direction = glm::vec3(0.0f, -1.0f, -1.0f);

        // Setup camera
        camera.location = glm::vec3(45.0f, 26.0f, 17.0f);
        camera.rotation = glm::vec2(-4.14f / 2.0f, -.6f);
        camera.exposure = 2.5f;

    }
    else if (sceneSelection == MEDIEVAL) {
        meshes.reserve(1);

        auto mesh = Atlas::ResourceManager<Atlas::Mesh::Mesh>::GetOrLoadResourceWithLoaderAsync(
            "medieval/scene.fbx", ModelLoader::LoadMesh, false, glm::mat4(1.0f), 2048
        );
        meshes.push_back(mesh);

        // Metalness is set to 0.9f
        //for (auto& material : mesh.data.materials) material.metalness = 0.0f;

        // Other scene related settings apart from the mesh
        directionalLight->intensity = 10.0f;
        directionalLight->GetVolumetric()->intensity = 0.08f;

        // Setup camera
        camera.location = glm::vec3(30.0f, 25.0f, 0.0f);
        camera.rotation = glm::vec2(-3.14f / 2.0f, 0.0f);

    }
    else if (sceneSelection == PICAPICA) {
        meshes.reserve(1);

        auto transform = glm::rotate(glm::mat4(1.0f), -3.14f / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        auto mesh = Atlas::ResourceManager<Atlas::Mesh::Mesh>::GetOrLoadResourceWithLoaderAsync(
            "pica pica/mesh/scene.gltf", ModelLoader::LoadMesh, false, transform, 2048
        );
        meshes.push_back(mesh);

        // Other scene related settings apart from the mesh
        directionalLight->intensity = 10.0f;
        directionalLight->GetVolumetric()->intensity = 0.08f;

        // Setup camera
        camera.location = glm::vec3(30.0f, 25.0f, 0.0f);
        camera.rotation = glm::vec2(-3.14f / 2.0f, 0.0f);
        camera.exposure = 1.0f;

    }
    else if (sceneSelection == SUBWAY) {
        meshes.reserve(1);

        auto mesh = Atlas::ResourceManager<Atlas::Mesh::Mesh>::GetOrLoadResourceWithLoaderAsync(
            "subway/scene.gltf", ModelLoader::LoadMesh, false, glm::mat4(1.0f), 2048
        );
        meshes.push_back(mesh);

        // Other scene related settings apart from the mesh
        directionalLight->intensity = 10.0f;
        directionalLight->GetVolumetric()->intensity = 0.08f;

        // Setup camera
        camera.location = glm::vec3(30.0f, 25.0f, 0.0f);
        camera.rotation = glm::vec2(-3.14f / 2.0f, 0.0f);
        camera.exposure = 1.0f;

    }
    else if (sceneSelection == MATERIALS) {
        meshes.reserve(1);

        auto transform = glm::scale(glm::vec3(8.0f));
        auto mesh = Atlas::ResourceManager<Atlas::Mesh::Mesh>::GetOrLoadResourceWithLoaderAsync(
            "material demo/materials.obj", ModelLoader::LoadMesh, false, transform, 2048
        );
        meshes.push_back(mesh);

        sky = Atlas::Texture::Cubemap("environment.hdr", 2048);
        probe = Atlas::Lighting::EnvironmentProbe(sky);
        scene->sky.probe = Atlas::CreateRef(probe);

        // Other scene related settings apart from the mesh
        directionalLight->intensity = 10.0f;
        directionalLight->GetVolumetric()->intensity = 0.0f;

        // Setup camera
        camera.location = glm::vec3(30.0f, 25.0f, 0.0f);
        camera.rotation = glm::vec2(-3.14f / 2.0f, 0.0f);
        camera.exposure = 1.0f;

        scene->sky.clouds->enable = false;
    }
    else if (sceneSelection == FOREST) {
        auto otherScene = Atlas::Loader::ModelLoader::LoadScene("forest/forest.gltf");
        otherScene->Update(&camera, 1.0f);

        CopyActors(otherScene);

        // Other scene related settings apart from the mesh
        directionalLight->intensity = 50.0f;
        directionalLight->GetVolumetric()->intensity = 0.08f;

        // Setup camera
        camera.location = glm::vec3(30.0f, 25.0f, 0.0f);
        camera.rotation = glm::vec2(-3.14f / 2.0f, 0.0f);
        camera.exposure = 1.0f;

    }
    else if (sceneSelection == EMERALDSQUARE) {
        auto otherScene = Atlas::Loader::ModelLoader::LoadScene("emeraldsquare/square.gltf", false, glm::mat4(1.0f), 1024);
        otherScene->Update(&camera, 1.0f);

        CopyActors(otherScene);

        // Other scene related settings apart from the mesh
        directionalLight->intensity = 10.0f;
        directionalLight->GetVolumetric()->intensity = 0.08f;

        // Setup camera
        camera.location = glm::vec3(30.0f, 25.0f, 0.0f);
        camera.rotation = glm::vec2(-3.14f / 2.0f, 0.0f);
        camera.exposure = 1.0f;
    }
    else if (sceneSelection == FLYINGWORLD) {
        meshes.reserve(1);

        auto mesh = Atlas::ResourceManager<Atlas::Mesh::Mesh>::GetOrLoadResourceWithLoaderAsync(
            "flying world/scene.gltf", ModelLoader::LoadMesh, false, glm::mat4(0.01f), 2048
        );
        meshes.push_back(mesh);

        // Metalness is set to 0.9f
        //for (auto& material : mesh.data.materials) material.metalness = 0.0f;

        // Other scene related settings apart from the mesh
        directionalLight->intensity = 50.0f;
        directionalLight->GetVolumetric()->intensity = 0.08f;

        // Setup camera
        camera.location = glm::vec3(30.0f, 25.0f, 0.0f);
        camera.rotation = glm::vec2(-3.14f / 2.0f, 0.0f);
        camera.exposure = 1.0f;

        scene->sky.clouds->minHeight = 700.0f;
        scene->sky.clouds->maxHeight = 1000.0f;
        scene->sky.clouds->densityMultiplier = 0.65f;
        scene->sky.clouds->heightStretch = 1.0f;

    }
    else if (sceneSelection == NEWSPONZA) {
        meshes.reserve(4);

        auto transform = glm::mat4(glm::scale(glm::mat4(1.0f), glm::vec3(4.0f)));
        auto mesh = Atlas::ResourceManager<Atlas::Mesh::Mesh>::GetOrLoadResourceWithLoaderAsync(
            "newsponza/main/NewSponza_Main_Blender_glTF.gltf", ModelLoader::LoadMesh, false, transform, 2048
        );
        meshes.push_back(mesh);
        mesh = Atlas::ResourceManager<Atlas::Mesh::Mesh>::GetOrLoadResourceWithLoaderAsync(
            "newsponza/candles/NewSponza_100sOfCandles_glTF_OmniLights.gltf", ModelLoader::LoadMesh, false, transform, 2048
        );
        meshes.push_back(mesh);
        mesh = Atlas::ResourceManager<Atlas::Mesh::Mesh>::GetOrLoadResourceWithLoaderAsync(
            "newsponza/curtains/NewSponza_Curtains_glTF.gltf", ModelLoader::LoadMesh, false, transform, 2048
        );
        meshes.push_back(mesh);
        mesh = Atlas::ResourceManager<Atlas::Mesh::Mesh>::GetOrLoadResourceWithLoaderAsync(
            "newsponza/ivy/NewSponza_IvyGrowth_glTF.gltf", ModelLoader::LoadMesh, false, transform, 2048
        );
        meshes.push_back(mesh);

        // Other scene related settings apart from the mesh
        directionalLight->direction = glm::vec3(0.0f, -1.0f, 0.33f);
        directionalLight->intensity = 100.0f;
        directionalLight->GetVolumetric()->intensity = 0.28f;

        // Setup camera
        camera.location = glm::vec3(30.0f, 25.0f, 0.0f);
        camera.rotation = glm::vec2(-3.14f / 2.0f, 0.0f);

    }

    // scene.sky.probe = std::make_shared<Atlas::Lighting::EnvironmentProbe>(sky);

    if (sceneSelection != FOREST && sceneSelection != EMERALDSQUARE) {
        auto meshCount = 0;
        for (auto &mesh: meshes) {
            if (meshCount == 10) {
                meshCount++;
                continue;
            }
            actors.push_back(Atlas::Actor::MovableMeshActor{mesh, glm::translate(glm::mat4(1.0f),
                glm::vec3(0.0f))});

            /*
            if (meshCount == 1) {
                for (int32_t i = 0; i < 20000; i++) {
                    auto x = (2.0f * Atlas::Common::Random::SampleFastUniformFloat() - 1.0f) * 100.0f;
                    auto y = (2.0f * Atlas::Common::Random::SampleFastUniformFloat() - 1.0f) * 100.0f;
                    auto z = (2.0f * Atlas::Common::Random::SampleFastUniformFloat() - 1.0f) * 100.0f;

                    actors.push_back(Atlas::Actor::MovableMeshActor{mesh, glm::translate(glm::mat4(1.0f),
                        glm::vec3(x, y, z))});
                }
            }
            */

            meshCount++;
        }
    }

    for (auto& actor : actors) {
        scene->Add(&actor);
    }

    camera.Update();
    scene->Update(&camera, 1.0f);

    // Reset input handlers
    keyboardHandler.Reset(&camera);
    mouseHandler.Reset(&camera);

    Atlas::Clock::ResetAverage();

    return successful;

}

void App::UnloadScene() {

    for (auto& actor : actors) scene->Remove(&actor);

    actors.clear();
    meshes.clear();

    actors.shrink_to_fit();
    meshes.shrink_to_fit();


    graphicsDevice->WaitForIdle();
    graphicsDevice->ForceMemoryCleanup();

}

void App::CheckLoadScene() {

    if (!scene->IsFullyLoaded() || loadingComplete)
        return;

    if (sceneSelection == NEWSPONZA) {
        for (auto& mesh : meshes) {
            mesh->data.colors.Clear();
        }
    }
    else if (sceneSelection == PICAPICA) {
        for (const auto& material : meshes.front()->data.materials)
            material->vertexColors = false;
    }
    else if (sceneSelection == EMERALDSQUARE) {
        for (const auto& mesh : meshes)
            for (const auto& material : mesh->data.materials)
                material->metalness = 0.0f;
    }


    auto sceneAABB = Atlas::Volume::AABB(glm::vec3(std::numeric_limits<float>::max()),
        glm::vec3(-std::numeric_limits<float>::max()));

    auto sceneActors = scene->GetMeshActors();
    for (auto& actor : sceneActors) {
        sceneAABB.Grow(actor->aabb);
    }

    for (auto& mesh : meshes) {
        mesh->invertUVs = true;
        mesh->cullBackFaces = true;
    }


    Atlas::Clock::ResetAverage();

    loadingComplete = true;

}

void App::SetResolution(int32_t width, int32_t height) {

    renderTarget.Resize(width, height);

}

void App::CopyActors(Atlas::Ref<Atlas::Scene::Scene> otherScene) {

    auto otherActors = otherScene->GetMeshActors();

    for (auto actor : otherActors) {

        actors.push_back(Atlas::Actor::MovableMeshActor{actor->mesh, actor->globalMatrix});

        delete actor;

    }

    auto otherMeshes = otherScene->GetMeshes();

    for (auto mesh : otherMeshes) {

        meshes.push_back(mesh);

    }

}

Atlas::EngineInstance* GetEngineInstance() {

    return new App();

}
