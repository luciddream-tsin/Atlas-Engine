#pragma once

#include "System.h"
#include "actor/MeshActor.h"
#include "actor/DecalActor.h"
#include "lighting/Light.h"

#include "graphics/Buffer.h"

#include <map>
#include <vector>

namespace Atlas {

    class RenderList {

    public:
        struct MeshInstances {
            size_t offset;
            size_t count;

        };

        enum class RenderPassType {
            Main = 0,
            Shadow = 1
        };

        struct Pass {
            RenderPassType type;

            Lighting::Light* light;
            uint32_t layer;

            std::map<size_t, std::vector<Actor::MeshActor*>> meshToActorMap;
            std::map<size_t, MeshInstances> meshToInstancesMap;
            std::map<size_t, ResourceHandle<Mesh::Mesh>> meshIdToMeshMap;
        };

        RenderList();

        void NewFrame();

        void NewMainPass();

        void NewShadowPass(Lighting::Light* light, uint32_t layer);

        Pass* GetMainPass();

        Pass* GetShadowPass(const Lighting::Light* light, const uint32_t layer);

        void Add(Actor::MeshActor *actor);

        void Update(Camera* camera);

        void FillBuffers();

        std::vector<mat3x4> currentActorMatrices;
        std::vector<mat3x4> lastActorMatrices;

        Ref<Graphics::MultiBuffer> currentMatricesBuffer;
        Ref<Graphics::MultiBuffer> lastMatricesBuffer;

        std::vector<Pass> passes;

    };

}