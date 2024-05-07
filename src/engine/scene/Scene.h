#pragma once

#include "../System.h"
#include "../actor/MeshActor.h"
#include "../terrain/Terrain.h"
#include "../lighting/Light.h"
#include "../lighting/Sky.h"
#include "../lighting/Fog.h"
#include "../lighting/IrradianceVolume.h"
#include "../lighting/AO.h"
#include "../lighting/Reflection.h"
#include "../lighting/VolumetricClouds.h"
#include "../lighting/SSS.h"
#include "../lighting/SSGI.h"
#include "../ocean/Ocean.h"
#include "../postprocessing/PostProcessing.h"
#include "../Decal.h"

#include "SceneNode.h"
#include "SpacePartitioning.h"


#include <unordered_map>

namespace Atlas {

    namespace Scene {

        class Scene : public SceneNode, public SpacePartitioning {

            friend class Renderer::MainRenderer;

        public:
            /**
             * Constructs a scene object.
             */
            Scene() : SceneNode(this, &rootMeshMap), SpacePartitioning(vec3(-2048.0f), vec3(2048.0f), 5)
                     {}

            Scene(vec3 min, vec3 max, int32_t depth = 5);

            ~Scene();

            Scene& operator=(const Scene& that);

            void Update(Camera *camera, float deltaTime);

            bool HasChanged();

            void SetMatrix() {}

            void Clear();

            std::vector<ResourceHandle<Mesh::Mesh>> GetMeshes();

            std::vector<Material*> GetMaterials();

            void WaitForResourceLoad();

            bool IsFullyLoaded();

            using SceneNode::Add;
            using SceneNode::Remove;

            Lighting::Sky sky;
            PostProcessing::PostProcessing postProcessing;

        private:

            std::unordered_map<size_t, RegisteredMesh> rootMeshMap;
            bool hasChanged = true;

        };

    }

}