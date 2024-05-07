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
#include "RTData.h"
#include "Vegetation.h"

#include <unordered_map>

namespace Atlas {

    namespace Scene {

        class Scene : public SceneNode  {

            friend class Renderer::MainRenderer;

        public:


            Scene(vec3 min, vec3 max, int32_t depth = 5);

            /**
             * Destructs a scene object.
             */
            ~Scene();


            Scene& operator=(const Scene& that);

            /**
             * Updates the scene based on the camera and the times passed.
             * @param camera The camera from whichs perspective the scene should be rendered later on.
             * @param deltaTime The time that has passed since the last update.
             */
            void Update(Camera *camera, float deltaTime);

            /**
             * Checks whether the scene has changed since the last update
             * @return True if scene has changed, false otherwise.
             */
            bool HasChanged();

            /**
             * Renamed from SceneNode: A scene shouldn't be transformed.
             */
            void SetMatrix() {}

            /**
             * Removes everything from the scene.
             */
            void Clear();

            /**
             * Returns all the scenes meshes
             * @return The meshes of the scene
             */
            std::vector<ResourceHandle<Mesh::Mesh>> GetMeshes();

            /**
             * Returns all the scenes materials
             * @return The materials of the scene
             * @note The material pointers are only valid until there
             * are material changes in meshes, the terrain, etc.
             */
            std::vector<Material*> GetMaterials();



            /**
             * Waits for all resources to be loaded that are in the scene
             */
            void WaitForResourceLoad();

            /**
             * Checks if all resources are loaded
             * @return
             */
            bool IsFullyLoaded();


            /**
             * To overload the Add and Remove methods we need to specify this
             * here. It would just rename the method instead.
             */
            using SceneNode::Add;
            using SceneNode::Remove;

            Lighting::Sky sky;
            PostProcessing::PostProcessing postProcessing;

        private:
            void UpdateBindlessIndexMaps();

            std::unordered_map<size_t, RegisteredMesh> rootMeshMap;
            std::unordered_map<Ref<Texture::Texture2D>, uint32_t> textureToBindlessIdx;
            std::unordered_map<size_t, uint32_t> meshIdToBindlessIdx;


            bool hasChanged = true;

        };

    }

}