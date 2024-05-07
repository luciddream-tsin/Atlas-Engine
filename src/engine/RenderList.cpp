#include <actor/MeshActor.h>
#include "RenderList.h"

#include "graphics/GraphicsDevice.h"

#include <glm/gtx/norm.hpp>

namespace Atlas {

    RenderList::RenderList()  {



    }

    void RenderList::NewFrame() {

        auto lastSize = currentActorMatrices.size();
        currentActorMatrices.clear();
        if (lastSize) currentActorMatrices.reserve(lastSize);

        lastSize = lastActorMatrices.size();
        lastActorMatrices.clear();
        if (lastSize) lastActorMatrices.reserve(lastSize);


        passes.clear();

    }

    void RenderList::NewMainPass() {

        Pass pass {
            .type = RenderPassType::Main,
            .light = nullptr,
            .layer = 0
        };

        passes.push_back(pass);

    }

    void RenderList::NewShadowPass(Lighting::Light *light, uint32_t layer) {

        Pass pass {
            .type = RenderPassType::Shadow,
            .light = light,
            .layer = layer
        };

        passes.push_back(pass);

    }


    RenderList::Pass* RenderList::GetMainPass() {

        for (auto& pass : passes) {
            if (pass.type == RenderPassType::Main) return &pass;
        }

        return nullptr;

    }

    RenderList::Pass* RenderList::GetShadowPass(const Lighting::Light *light, const uint32_t layer) {

        for (auto& pass : passes) {
            if (pass.type == RenderPassType::Shadow &&
                pass.light == light && pass.layer == layer) return &pass;
        }

        return nullptr;

    }

    void RenderList::Add(Actor::MeshActor *actor) {

        auto& pass = passes.back();
        auto& meshToActorMap = pass.meshToActorMap;

        if (!actor->mesh.IsLoaded())
            return;

        auto id = actor->mesh.GetID();

        if (!meshToActorMap.contains(id)) {
            auto& meshIdToMeshMap = pass.meshIdToMeshMap;

            meshToActorMap[id] = { actor };
            meshIdToMeshMap[id] = actor->mesh;
        }
        else {
            meshToActorMap[id].push_back(actor);
        }

    }

    void RenderList::Update(Camera* camera) {

        auto cameraLocation = camera->GetLocation();

        auto& pass = passes.back();
        auto type = pass.type;
        auto& meshToActorMap = pass.meshToActorMap;
        auto& meshToInstancesMap = pass.meshToInstancesMap;
        auto& meshIdToMeshMap = pass.meshIdToMeshMap;

        size_t maxActorCount = 0;

        for (auto& [meshId, actors] : meshToActorMap) {
            auto mesh = meshIdToMeshMap[meshId];
            if (!mesh->castShadow && type == RenderPassType::Shadow)
                continue;
            maxActorCount += actors.size();
        }

        currentActorMatrices.reserve(maxActorCount);
        lastActorMatrices.reserve(maxActorCount);

        for (auto& [meshId, actors] : meshToActorMap) {
            auto mesh = meshIdToMeshMap[meshId];
            if (!actors.size()) continue;
            if (!mesh->castShadow && type == RenderPassType::Shadow) continue;

            auto needsHistory = mesh->mobility != Mesh::MeshMobility::Stationary
                && type != RenderPassType::Shadow;

            MeshInstances instances;

            instances.offset = currentActorMatrices.size();

            {
                for (auto actor : actors) {
                    currentActorMatrices.push_back(glm::transpose(actor->globalMatrix));
                    if (needsHistory) {
                        lastActorMatrices.push_back(glm::transpose(actor->lastGlobalMatrix));
                    }
                    else {
                        // For now push back anyways
                        lastActorMatrices.push_back(glm::transpose(actor->globalMatrix));
                    }
                }
            }

            instances.count = currentActorMatrices.size() - instances.offset;
            meshToInstancesMap[meshId] = instances;

        }

    }

    void RenderList::FillBuffers() {

        auto device = Graphics::GraphicsDevice::DefaultDevice;

        if (!currentMatricesBuffer || currentMatricesBuffer->size < sizeof(mat3x4) * currentActorMatrices.size()) {
            auto newSize = currentMatricesBuffer != nullptr ? currentMatricesBuffer->size * 2 :
                           sizeof(mat3x4) * currentActorMatrices.size();
            newSize = std::max(std::max(newSize, size_t(1)), sizeof(mat3x4) * currentActorMatrices.size());
            auto bufferDesc = Graphics::BufferDesc {
                .usageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                .domain = Graphics::BufferDomain::Host,
                .size = newSize
            };
            if (newSize > 0) currentMatricesBuffer = device->CreateMultiBuffer(bufferDesc);
            if (newSize > 0) lastMatricesBuffer = device->CreateMultiBuffer(bufferDesc);
        }



        // Probably better to use a device local buffer since we upload once per frame
        if (currentActorMatrices.size() > 0)
            currentMatricesBuffer->SetData(currentActorMatrices.data(), 0, currentActorMatrices.size() * sizeof(mat3x4));
        if (lastActorMatrices.size() > 0)
            lastMatricesBuffer->SetData(lastActorMatrices.data(), 0, lastActorMatrices.size() * sizeof(mat3x4));

    }

}