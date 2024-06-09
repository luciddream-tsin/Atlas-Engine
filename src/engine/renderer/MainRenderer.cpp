#include "MainRenderer.h"
#include "helper/GeometryHelper.h"
#include "helper/HaltonSequence.h"

#include "../common/Packing.h"
#include "../Clock.h"

#define FEATURE_BASE_COLOR_MAP (1 << 1)
#define FEATURE_OPACITY_MAP (1 << 2)
#define FEATURE_NORMAL_MAP (1 << 3)
#define FEATURE_ROUGHNESS_MAP (1 << 4)
#define FEATURE_METALNESS_MAP (1 << 5)
#define FEATURE_AO_MAP (1 << 6)
#define FEATURE_TRANSMISSION (1 << 7)
#define FEATURE_VERTEX_COLORS (1 << 8)

namespace Atlas {

    namespace Renderer {

        void MainRenderer::Init(Graphics::GraphicsDevice *device) {

            this->device = device;

            CreateGlobalDescriptorSetLayout();

            auto uniformBufferDesc = Graphics::BufferDesc {
                .usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                .domain = Graphics::BufferDomain::Host,
                .hostAccess = Graphics::BufferHostAccess::Sequential,
                .size = sizeof(GlobalUniforms),
            };
            globalUniformBuffer = device->CreateMultiBuffer(uniformBufferDesc);


            opaqueRenderer.Init(device);
            shadowRenderer.Init(device);
            directLightRenderer.Init(device);

            textRenderer.Init(device);
            textureRenderer.Init(device);

        }

        void MainRenderer::RenderScene(Viewport* viewport, RenderTarget* target, Camera* camera, 
            Scene::Scene* scene, Texture::Texture2D* texture, RenderBatch* batch) {

            if (!device->swapChain->isComplete) 
                return;

            auto commandList = device->GetCommandList(Graphics::QueueType::GraphicsQueue);

            commandList->BeginCommands();

            {
                // Even if there is no TAA we need to update the jitter for other techniques
                // E.g. the reflections and ambient occlusion use reprojection
                camera->Jitter(vec2(0.0f));
            }

            Graphics::Profiler::BeginThread("Main renderer", commandList);
            Graphics::Profiler::BeginQuery("Render scene");

            FillRenderList(scene, camera);

            std::vector<PackedMaterial> materials;
            std::unordered_map<void*, uint16_t> materialMap;

            PrepareMaterials(scene, materials, materialMap);


            SetUniforms(scene, camera);

            commandList->BindBuffer(globalUniformBuffer, 0, 3);


            // Bind before any shadows etc. are rendered, this is a shared buffer for all these passes
            commandList->BindBuffer(renderList.currentMatricesBuffer, 1, 1);
            commandList->BindBuffer(renderList.lastMatricesBuffer, 1, 2);



            {
                Graphics::Profiler::BeginQuery("Main render pass");

                commandList->BeginRenderPass(target->gBufferRenderPass, target->gBufferFrameBuffer, true);

                opaqueRenderer.Render(viewport, target, camera, scene, commandList, &renderList, materialMap);
                commandList->EndRenderPass();

                Graphics::Profiler::EndQuery();
            }


            {

                Graphics::Profiler::BeginQuery("Main");

                auto shaderConfig = ShaderConfig {
                        { "postprocessing.vsh", VK_SHADER_STAGE_VERTEX_BIT },
                        { "postprocessing.fsh", VK_SHADER_STAGE_FRAGMENT_BIT }
                };
                auto pipelineDesc = Graphics::GraphicsPipelineDesc {
                        .swapChain = device->swapChain
                };

                std::vector<std::string> macros;

                auto pipelineConfig =  PipelineConfig(shaderConfig, pipelineDesc, macros);

                // We can't return here because of the queries
                if (device->swapChain->isComplete) {
                    commandList->BeginRenderPass(device->swapChain, true);

                    pipelineConfig.ManageMacro("FILMIC_TONEMAPPING", false);
                    pipelineConfig.ManageMacro("VIGNETTE", false);
                    pipelineConfig.ManageMacro("CHROMATIC_ABERRATION", false);
                    pipelineConfig.ManageMacro("FILM_GRAIN", false);

                    auto pipeline = PipelineManager::GetPipeline(pipelineConfig);
                    commandList->BindPipeline(pipeline);
                    //SetUniforms(camera, scene);
                    {
                        target->targetData.normalTexture->Bind(commandList, 3, 0);
                    }

                    commandList->Draw(6, 1, 0, 0);

                    commandList->EndRenderPass();
                }

                Graphics::Profiler::EndQuery();
            }



            Graphics::Profiler::EndQuery();
            Graphics::Profiler::EndThread();

            commandList->EndCommands();
            device->SubmitCommandList(commandList);

        }



        void MainRenderer::Update() {
            textRenderer.Update();
            frameCount++;
        }

        void MainRenderer::CreateGlobalDescriptorSetLayout() {

            if (!device->support.bindless)
                return;

            auto samplerDesc = Graphics::SamplerDesc {
                .filter = VK_FILTER_LINEAR,
                .mode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .maxLod = 12,
                .anisotropicFiltering = true
            };
            globalSampler = device->CreateSampler(samplerDesc);

            auto layoutDesc = Graphics::DescriptorSetLayoutDesc{
                .bindings = {
                    {
                        .bindingIdx = 0, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        .descriptorCount = 8192, .bindless = true
                    },
                    {
                        .bindingIdx = 1, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        .descriptorCount = 8192, .bindless = true
                    },
                    {
                        .bindingIdx = 2, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        .descriptorCount = 8192, .bindless = true
                    },
                    {
                        .bindingIdx = 3, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        .descriptorCount = 8192, .bindless = true
                    },
                    {
                        .bindingIdx = 4, .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                        .descriptorCount = 16384, .bindless = true
                    }
                },
                .bindingCount = 5
            };
            globalDescriptorSetLayout = device->CreateDescriptorSetLayout(layoutDesc);

            PipelineManager::OverrideDescriptorSetLayout(globalDescriptorSetLayout, 0);

        }

        void MainRenderer::SetUniforms(Scene::Scene *scene, Camera *camera) {

            auto globalUniforms = GlobalUniforms {
                .vMatrix = camera->viewMatrix,
                .pMatrix = camera->projectionMatrix,
                .ivMatrix = camera->invViewMatrix,
                .ipMatrix = camera->invProjectionMatrix,
                .pvMatrixLast = camera->GetLastJitteredMatrix(),
                .pvMatrixCurrent = camera->projectionMatrix * camera->viewMatrix,
                .jitterLast = camera->GetLastJitter(),
                .jitterCurrent = camera->GetJitter(),
                .cameraLocation = vec4(camera->location, 0.0f),
                .cameraDirection = vec4(camera->direction, 0.0f),
                .cameraUp = vec4(camera->up, 0.0f),
                .cameraRight = vec4(camera->right, 0.0f),
                .planetCenter = vec4(scene->sky.planetCenter, 0.0f),
                .planetRadius = scene->sky.planetRadius,
                .time = Clock::Get(),
                .deltaTime = Clock::GetDelta(),
                .frameCount = frameCount
            };

            auto frustumPlanes = camera->frustum.GetPlanes();
            std::copy(frustumPlanes.begin(), frustumPlanes.end(), &globalUniforms.frustumPlanes[0]);

            globalUniformBuffer->SetData(&globalUniforms, 0, sizeof(GlobalUniforms));


        }

        void MainRenderer::PrepareMaterials(Scene::Scene* scene, std::vector<PackedMaterial>& materials,
            std::unordered_map<void*, uint16_t>& materialMap) {

            auto sceneMaterials = scene->GetMaterials();

            uint16_t idx = 0;

            for (auto material : sceneMaterials) {
                PackedMaterial packed;

                packed.baseColor = Common::Packing::PackUnsignedVector3x10_1x2(vec4(Common::ColorConverter::ConvertSRGBToLinear(material->baseColor), 0.0f));
                packed.emissiveColor = Common::Packing::PackUnsignedVector3x10_1x2(vec4(Common::ColorConverter::ConvertSRGBToLinear(material->emissiveColor), 0.0f));
                packed.transmissionColor = Common::Packing::PackUnsignedVector3x10_1x2(vec4(Common::ColorConverter::ConvertSRGBToLinear(material->transmissiveColor), 0.0f));

                packed.emissiveIntensityTiling = glm::packHalf2x16(vec2(material->emissiveIntensity, material->tiling));

                vec4 data0, data1, data2;

                data0.x = material->opacity;
                data0.y = material->roughness;
                data0.z = material->metalness;

                data1.x = material->ao;
                data1.y = material->HasNormalMap() ? material->normalScale : 0.0f;
                data1.z = material->HasDisplacementMap() ? material->displacementScale : 0.0f;

                data2.x = material->reflectance;
                // Note used
                data2.y = 0.0f;
                data2.z = 0.0f;

                packed.data0 = Common::Packing::PackUnsignedVector3x10_1x2(data0);
                packed.data1 = Common::Packing::PackUnsignedVector3x10_1x2(data1);
                packed.data2 = Common::Packing::PackUnsignedVector3x10_1x2(data2);

                packed.features = 0;

                packed.features |= material->HasBaseColorMap() ? FEATURE_BASE_COLOR_MAP : 0;
                packed.features |= material->HasOpacityMap() ? FEATURE_OPACITY_MAP : 0;
                packed.features |= material->HasNormalMap() ? FEATURE_NORMAL_MAP : 0;
                packed.features |= material->HasRoughnessMap() ? FEATURE_ROUGHNESS_MAP : 0;
                packed.features |= material->HasMetalnessMap() ? FEATURE_METALNESS_MAP : 0;
                packed.features |= material->HasAoMap() ? FEATURE_AO_MAP : 0;
                packed.features |= glm::length(material->transmissiveColor) > 0.0f ? FEATURE_TRANSMISSION : 0;
                packed.features |= material->vertexColors ? FEATURE_VERTEX_COLORS : 0;

                materials.push_back(packed);

                materialMap[material] = idx++;
            }
            
        }


        void MainRenderer::FillRenderList(Scene::Scene *scene, Atlas::Camera *camera) {

            renderList.NewFrame();
            renderList.NewMainPass();

            scene->GetRenderList(camera->frustum, renderList);
            renderList.Update(camera);

            auto lights = scene->GetLights();

            if (scene->sky.sun) {
                lights.push_back(scene->sky.sun.get());
            }

            for (auto light : lights) {
                if (!light->GetShadow())
                    continue;
                if (!light->GetShadow()->update)
                    continue;

                auto componentCount = light->GetShadow()->longRange ?
                    light->GetShadow()->componentCount - 1 :
                    light->GetShadow()->componentCount;

                for (int32_t i = 0; i < componentCount; i++) {
                    auto component = &light->GetShadow()->components[i];
                    auto frustum = Volume::Frustum(component->frustumMatrix);

                    renderList.NewShadowPass(light, i);
                    scene->GetRenderList(frustum, renderList);
                    renderList.Update(camera);
                }

            }

            renderList.FillBuffers();

        }

    }

}
