#pragma once

#include "../System.h"
#include "../graphics/GraphicsDevice.h"

#include "RenderBatch.h"

#include "OpaqueRenderer.h"
#include "ShadowRenderer.h"
#include "DirectLightRenderer.h"
#include "PointLightRenderer.h"

#include "TextRenderer.h"

#include "TextureRenderer.h"

namespace Atlas {

    namespace Renderer {

        class MainRenderer {

        public:
            MainRenderer() = default;

            void Init(Graphics::GraphicsDevice* device);

            /**
             *
             * @param window
             * @param target
             * @param camera
             * @param scene
             */
            void RenderScene(Viewport* viewport, RenderTarget* target, Camera* camera,
                Scene::Scene* scene, Texture::Texture2D* texture = nullptr, 
                RenderBatch* batch = nullptr);





            /**
             * Update of the renderer
             * @warning Must be called every frame
             */
            void Update();

            TextRenderer textRenderer;
            TextureRenderer textureRenderer;

        private:
            struct PackedMaterial {

                int32_t baseColor;
                int32_t emissiveColor;
                int32_t transmissionColor;

                uint32_t emissiveIntensityTiling;

                int32_t data0;
                int32_t data1;
                int32_t data2;

                int32_t features;

            };

            struct alignas(16) GlobalUniforms {
                vec4 frustumPlanes[6];
                mat4 vMatrix;
                mat4 pMatrix;
                mat4 ivMatrix;
                mat4 ipMatrix;
                mat4 pvMatrixLast;
                mat4 pvMatrixCurrent;
                vec2 jitterLast;
                vec2 jitterCurrent;
                vec4 cameraLocation;
                vec4 cameraDirection;
                vec4 cameraUp;
                vec4 cameraRight;
                vec4 planetCenter;
                float planetRadius;
                float time;
                float deltaTime;
                uint32_t frameCount;
            };

            struct alignas(16) DDGIUniforms {
                vec4 volumeMin;
                vec4 volumeMax;
                ivec4 volumeProbeCount;
                vec4 cellSize;

                float volumeBias;

                int32_t volumeIrradianceRes;
                int32_t volumeMomentsRes;

                uint32_t rayCount;
                uint32_t inactiveRayCount;

                float hysteresis;

                float volumeGamma;
                float volumeStrength;

                float depthSharpness;
                int optimizeProbes;

                int32_t volumeEnabled;
            };

            void CreateGlobalDescriptorSetLayout();

            void SetUniforms(Scene::Scene* scene, Camera* camera);

            void PrepareMaterials(Scene::Scene* scene, std::vector<PackedMaterial>& materials,
                std::unordered_map<void*, uint16_t>& materialMap);

             void FillRenderList(Scene::Scene* scene, Camera* camera);


            Graphics::GraphicsDevice* device = nullptr;

            Texture::Texture2D dfgPreintegrationTexture;

            Ref<Graphics::MultiBuffer> globalUniformBuffer;
            Ref<Graphics::DescriptorSetLayout> globalDescriptorSetLayout;
            Ref<Graphics::Sampler> globalSampler;


            OpaqueRenderer opaqueRenderer;
            ShadowRenderer shadowRenderer;
            DirectLightRenderer directLightRenderer;


            RenderList renderList;

            uint32_t frameCount = 0;

        };

    }

}