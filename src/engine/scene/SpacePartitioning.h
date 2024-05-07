#pragma once

#include "../System.h"
#include "../RenderList.h"
#include "../volume/Octree.h"

#include "../actor/StaticMeshActor.h"
#include "../actor/MovableMeshActor.h"
#include "../actor/DecalActor.h"
#include "../actor/AudioActor.h"

#include "../lighting/Light.h"

namespace Atlas {

    namespace Scene {

        class SpacePartitioning {

        public:
            SpacePartitioning(vec3 min, vec3 max, int32_t depth);

            SpacePartitioning& operator=(const SpacePartitioning& that);

            void Add(Actor::MovableMeshActor* actor);

            void Remove(Actor::MovableMeshActor* actor);

            void Add(Actor::StaticMeshActor* actor);

            void Remove(Actor::StaticMeshActor* actor);

            void Add(Lighting::Light* light);

            void Remove(Lighting::Light* light);

            void GetRenderList(Volume::Frustum frustum, RenderList& renderList);

            std::vector<Actor::Actor*> GetActors();

            std::vector<Actor::MeshActor*> GetMeshActors();

            std::vector<Actor::StaticMeshActor*> GetStaticMeshActors(Volume::AABB aabb);
            std::vector<Actor::MovableMeshActor*> GetMovableMeshActors(Volume::AABB aabb);

            std::vector<Lighting::Light*> GetLights();

            void Clear();

        private:
            Volume::AABB aabb;
            
            Volume::Octree<Actor::MovableMeshActor*> movableMeshOctree;
            Volume::Octree<Actor::StaticMeshActor*> staticMeshOctree;

            std::vector<Lighting::Light*> lights;

        };

    }

}