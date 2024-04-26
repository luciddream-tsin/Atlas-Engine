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

            // PreintegrateBRDF();

            auto uniformBufferDesc = Graphics::BufferDesc {
                    .usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    .domain = Graphics::BufferDomain::Host,
                    .hostAccess = Graphics::BufferHostAccess::Sequential,
                    .size = sizeof(GlobalUniforms),
            };
            globalUniformBuffer = device->CreateMultiBuffer(uniformBufferDesc);


            opaqueRenderer.Init(device);
            shadowRenderer.Init(device);
            downscaleRenderer.Init(device);
            // sssRenderer.Init(device);
            directLightRenderer.Init(device);

            textRenderer.Init(device);
            textureRenderer.Init(device);

        }

        void MainRenderer::RenderScene(Ref<Viewport> viewport, Ref<RenderTarget> target, Ref<Scene::Scene> scene,
                                       Ref<PrimitiveBatch> batch, Texture::Texture2D* texture) {

            if (!device->swapChain->isComplete || !scene->HasMainCamera())
                return;

            auto& camera = scene->GetMainCamera();
            auto commandList = device->GetCommandList(Graphics::QueueType::GraphicsQueue);

            commandList->BeginCommands();

            camera.Jitter(vec2(0.0f));

            Graphics::Profiler::BeginThread("Main renderer", commandList);
            Graphics::Profiler::BeginQuery("Render scene");

            FillRenderList(scene, camera);

            std::vector<PackedMaterial> materials;
            std::unordered_map<void*, uint16_t> materialMap;

            PrepareMaterials(scene, materials, materialMap);

            SetUniforms(scene, camera);

            commandList->BindBuffer(globalUniformBuffer, 1, 31);
            commandList->BindImage(dfgPreintegrationTexture.image, dfgPreintegrationTexture.sampler, 1, 12);
            commandList->BindSampler(globalSampler, 1, 13);


            auto materialBufferDesc = Graphics::BufferDesc {
                    .usageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                    .domain = Graphics::BufferDomain::Host,
                    .hostAccess = Graphics::BufferHostAccess::Sequential,
                    .data = materials.data(),
                    .size = sizeof(PackedMaterial) * glm::max(materials.size(), size_t(1)),
            };
            auto materialBuffer = device->CreateBuffer(materialBufferDesc);
            commandList->BindBuffer(materialBuffer, 1, 14);

            // Bind before any shadows etc. are rendered, this is a shared buffer for all these passes
            commandList->BindBuffer(renderList.currentMatricesBuffer, 1, 1);
            commandList->BindBuffer(renderList.lastMatricesBuffer, 1, 2);


            {
                shadowRenderer.Render(target, scene, commandList, &renderList);
            }

            {
                VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                VkAccessFlags access = VK_ACCESS_SHADER_READ_BIT;

                std::vector<Graphics::BufferBarrier> bufferBarriers;
                std::vector<Graphics::ImageBarrier> imageBarriers;

                auto lightSubset = scene->GetSubset<LightComponent>();

                for (auto& lightEntity : lightSubset) {
                    auto& light = lightEntity.GetComponent<LightComponent>();
                    if (!light.shadow || !light.shadow->update)
                        continue;

                    auto shadow = light.shadow;
                    imageBarriers.push_back({ shadow->useCubemap ?
                                              shadow->cubemap.image : shadow->maps.image, layout, access });
                }
                commandList->PipelineBarrier(imageBarriers, bufferBarriers, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
            }

            {
                Graphics::Profiler::BeginQuery("Main render pass");

                commandList->BeginRenderPass(target->gBufferRenderPass, target->gBufferFrameBuffer, true);

                opaqueRenderer.Render(target, scene, commandList, &renderList, materialMap);

                commandList->EndRenderPass();

                Graphics::Profiler::EndQuery();
            }


            auto targetData = target->GetData(FULL_RES);

            commandList->BindImage(targetData->baseColorTexture->image, targetData->baseColorTexture->sampler, 1, 3);
            commandList->BindImage(targetData->normalTexture->image, targetData->normalTexture->sampler, 1, 4);
            commandList->BindImage(targetData->geometryNormalTexture->image, targetData->geometryNormalTexture->sampler, 1, 5);
            commandList->BindImage(targetData->roughnessMetallicAoTexture->image, targetData->roughnessMetallicAoTexture->sampler, 1, 6);
            commandList->BindImage(targetData->materialIdxTexture->image, targetData->materialIdxTexture->sampler, 1, 7);
            commandList->BindImage(targetData->depthTexture->image, targetData->depthTexture->sampler, 1, 8);

            {
                auto rtData = target->GetHistoryData(FULL_RES);

                VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                VkAccessFlags access = VK_ACCESS_SHADER_READ_BIT;

                std::vector<Graphics::BufferBarrier> bufferBarriers;
                std::vector<Graphics::ImageBarrier> imageBarriers = {
                        {rtData->baseColorTexture->image, layout, access},
                        {rtData->depthTexture->image, layout, access},
                        {rtData->normalTexture->image, layout, access},
                        {rtData->geometryNormalTexture->image, layout, access},
                        {rtData->roughnessMetallicAoTexture->image, layout, access},
                        {rtData->offsetTexture->image, layout, access},
                        {rtData->materialIdxTexture->image, layout, access},
                        {rtData->stencilTexture->image, layout, access},
                        {rtData->velocityTexture->image, layout, access},
                        {target->oceanStencilTexture.image, layout, access},
                        {target->oceanDepthTexture.image, layout, access}
                };

                commandList->PipelineBarrier(imageBarriers, bufferBarriers, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
            }


            downscaleRenderer.Downscale(target, commandList);
            // sssRenderer.Render(target, scene, commandList);

            {
                Graphics::Profiler::BeginQuery("Lighting pass");

                commandList->ImageMemoryBarrier(target->lightingTexture.image, VK_IMAGE_LAYOUT_GENERAL,
                                                VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);

                directLightRenderer.Render(target, scene, commandList);

                commandList->ImageMemoryBarrier(target->lightingTexture.image, VK_IMAGE_LAYOUT_GENERAL,
                                                VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);


                Graphics::ImageBarrier outBarrier(target->lightingTexture.image,
                                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT);

                commandList->ImageMemoryBarrier(outBarrier, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

                Graphics::Profiler::EndQuery();
            }
            {

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
                            target->lightingTexture.Bind(commandList, 3, 0);
                        }

                        commandList->Draw(6, 1, 0, 0);

                        commandList->EndRenderPass();
                    }

                    Graphics::Profiler::EndQuery();
                }
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
                                    .bindingIdx = 3, .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                    .descriptorCount = 16384, .bindless = true
                            }
                    },
                    .bindingCount = 4
            };
            globalDescriptorSetLayout = device->CreateDescriptorSetLayout(layoutDesc);

            PipelineManager::OverrideDescriptorSetLayout(globalDescriptorSetLayout, 0);

        }

        void MainRenderer::SetUniforms(Ref<Scene::Scene> scene, const CameraComponent& camera) {

            auto globalUniforms = GlobalUniforms {
                    .vMatrix = camera.viewMatrix,
                    .pMatrix = camera.projectionMatrix,
                    .ivMatrix = camera.invViewMatrix,
                    .ipMatrix = camera.invProjectionMatrix,
                    .pvMatrixLast = camera.GetLastJitteredMatrix(),
                    .pvMatrixCurrent = camera.projectionMatrix * camera.viewMatrix,
                    .jitterLast = camera.GetLastJitter(),
                    .jitterCurrent = camera.GetJitter(),
                    .cameraLocation = vec4(camera.location, 0.0f),
                    .cameraDirection = vec4(camera.direction, 0.0f),
                    .cameraUp = vec4(camera.up, 0.0f),
                    .cameraRight = vec4(camera.right, 0.0f),
                    .planetCenter = vec4(scene->sky.planetCenter, 0.0f),
                    .windDir = glm::normalize(scene->wind.direction),
                    .windSpeed = scene->wind.speed,
                    .planetRadius = scene->sky.planetRadius,
                    .time = Clock::Get(),
                    .deltaTime = Clock::GetDelta(),
                    .frameCount = frameCount
            };

            auto frustumPlanes = camera.frustum.GetPlanes();
            std::copy(frustumPlanes.begin(), frustumPlanes.end(), &globalUniforms.frustumPlanes[0]);

            globalUniformBuffer->SetData(&globalUniforms, 0, sizeof(GlobalUniforms));

            auto meshes = scene->GetMeshes();
            for (auto& mesh : meshes) {
                if (!mesh.IsLoaded() || !mesh->impostor) continue;

                auto impostor = mesh->impostor;
                Mesh::Impostor::ImpostorInfo impostorInfo = {
                        .center = vec4(impostor->center, 1.0f),
                        .radius = impostor->radius,
                        .views = impostor->views,
                        .cutoff = impostor->cutoff,
                        .mipBias = impostor->mipBias
                };

                impostor->impostorInfoBuffer.SetData(&impostorInfo, 0);
            }

        }

        void MainRenderer::PrepareMaterials(Ref<Scene::Scene> scene, std::vector<PackedMaterial>& materials,
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

            auto meshes = scene->GetMeshes();

            for (auto mesh : meshes) {
                if (!mesh.IsLoaded())
                    continue;

                auto impostor = mesh->impostor;

                if (!impostor)
                    continue;

                PackedMaterial packed;

                packed.baseColor = Common::Packing::PackUnsignedVector3x10_1x2(vec4(1.0f));
                packed.emissiveColor = Common::Packing::PackUnsignedVector3x10_1x2(vec4(0.0f));
                packed.transmissionColor = Common::Packing::PackUnsignedVector3x10_1x2(vec4(Common::ColorConverter::ConvertSRGBToLinear(impostor->transmissiveColor), 1.0f));

                vec4 data0, data1, data2;

                data0.x = 1.0f;
                data0.y = 1.0f;
                data0.z = 1.0f;

                data1.x = 1.0f;
                data1.y = 0.0f;
                data1.z = 0.0f;

                data2.x = 0.5f;
                // Note used
                data2.y = 0.0f;
                data2.z = 0.0f;

                packed.data0 = Common::Packing::PackUnsignedVector3x10_1x2(data0);
                packed.data1 = Common::Packing::PackUnsignedVector3x10_1x2(data1);
                packed.data2 = Common::Packing::PackUnsignedVector3x10_1x2(data2);

                packed.features = 0;

                packed.features |= FEATURE_BASE_COLOR_MAP |
                                   FEATURE_ROUGHNESS_MAP | FEATURE_METALNESS_MAP | FEATURE_AO_MAP;
                packed.features |= glm::length(impostor->transmissiveColor) > 0.0f ? FEATURE_TRANSMISSION : 0;

                materials.push_back(packed);

                materialMap[impostor.get()] =  idx++;
            }


        }

        void MainRenderer::PrepareBindlessData(Ref<Scene::Scene> scene, std::vector<Ref<Graphics::Image>>& images,
                                               std::vector<Ref<Graphics::Buffer>>& blasBuffers, std::vector<Ref<Graphics::Buffer>>& triangleBuffers,
                                               std::vector<Ref<Graphics::Buffer>>& bvhTriangleBuffers, std::vector<Ref<Graphics::Buffer>>& triangleOffsetBuffers) {

            if (!device->support.bindless)
                return;

            blasBuffers.resize(scene->meshIdToBindlessIdx.size());
            triangleBuffers.resize(scene->meshIdToBindlessIdx.size());
            bvhTriangleBuffers.resize(scene->meshIdToBindlessIdx.size());
            triangleOffsetBuffers.resize(scene->meshIdToBindlessIdx.size());

            for (const auto& [meshId, idx] : scene->meshIdToBindlessIdx) {
                if (!scene->registeredMeshes.contains(meshId)) continue;

                const auto& mesh = scene->registeredMeshes[meshId].resource;

                auto blasBuffer = mesh->blasNodeBuffer.Get();
                auto triangleBuffer = mesh->triangleBuffer.Get();
                auto bvhTriangleBuffer = mesh->bvhTriangleBuffer.Get();
                auto triangleOffsetBuffer = mesh->triangleOffsetBuffer.Get();

                        AE_ASSERT(triangleBuffer != nullptr);

                blasBuffers[idx] = blasBuffer;
                triangleBuffers[idx] = triangleBuffer;
                bvhTriangleBuffers[idx] = bvhTriangleBuffer;
                triangleOffsetBuffers[idx] = triangleOffsetBuffer;
            }

            images.resize(scene->textureToBindlessIdx.size());

            for (const auto& [texture, idx] : scene->textureToBindlessIdx) {

                images[idx] = texture->image;

            }

        }

        void MainRenderer::FillRenderList(Ref<Scene::Scene> scene, const CameraComponent& camera) {

            renderList.NewFrame(scene);
            renderList.NewMainPass();

            scene->GetRenderList(camera.frustum, renderList);
            renderList.Update(camera.GetLocation());

            auto lightSubset = scene->GetSubset<LightComponent>();

            for (auto& lightEntity : lightSubset) {

                auto& light = lightEntity.GetComponent<LightComponent>();
                if (!light.shadow || !light.shadow->update)
                    continue;

                auto& shadow = light.shadow;

                auto componentCount = shadow->longRange ?
                                      shadow->componentCount - 1 : shadow->componentCount;

                for (int32_t i = 0; i < componentCount; i++) {
                    auto component = &shadow->components[i];
                    auto frustum = Volume::Frustum(component->frustumMatrix);

                    renderList.NewShadowPass(lightEntity, i);
                    scene->GetRenderList(frustum, renderList);
                    renderList.Update(camera.GetLocation());
                }

            }

            renderList.FillBuffers();

        }

        void MainRenderer::PreintegrateBRDF() {

            auto pipelineConfig = PipelineConfig("brdf/preintegrateDFG.csh");
            auto computePipeline = PipelineManager::GetPipeline(pipelineConfig);

            const int32_t res = 256;
            dfgPreintegrationTexture = Texture::Texture2D(res, res, VK_FORMAT_R16G16B16A16_SFLOAT,
                                                          Texture::Wrapping::ClampToEdge, Texture::Filtering::Linear);

            auto commandList = device->GetCommandList(Graphics::QueueType::GraphicsQueue, true);

            commandList->BeginCommands();
            commandList->BindPipeline(computePipeline);

            auto barrier = Graphics::ImageBarrier(VK_IMAGE_LAYOUT_GENERAL,
                                                  VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
            commandList->ImageMemoryBarrier(barrier.Update(dfgPreintegrationTexture.image),
                                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

            uint32_t groupCount = res / 8;

            commandList->BindImage(dfgPreintegrationTexture.image, 3, 0);
            commandList->Dispatch(groupCount, groupCount, 1);

            barrier = Graphics::ImageBarrier(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                             VK_ACCESS_SHADER_READ_BIT);
            commandList->ImageMemoryBarrier(barrier.Update(dfgPreintegrationTexture.image),
                                            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

            commandList->EndCommands();
            device->FlushCommandList(commandList);

        }

    }

}
