#include "SpacePartitioning.h"

#include <glm/gtx/norm.hpp>
#include <algorithm>

namespace Atlas {

    namespace Scene {

        SpacePartitioning::SpacePartitioning(vec3 min, vec3 max, int32_t depth) : 
            aabb(min, max) {

            staticMeshOctree = Volume::Octree<Actor::StaticMeshActor*>(aabb, depth);
            movableMeshOctree = Volume::Octree<Actor::MovableMeshActor*>(aabb, depth);
        }

        SpacePartitioning& SpacePartitioning::operator=(const SpacePartitioning& that) {

            if (this != &that) {

                aabb = that.aabb;

                staticMeshOctree = that.staticMeshOctree;
                movableMeshOctree = that.movableMeshOctree;

                lights = that.lights;

            }

            return *this;

        }

        void SpacePartitioning::Add(Actor::MovableMeshActor* actor) {

            movableMeshOctree.Insert(actor, actor->aabb);

        }

        void SpacePartitioning::Remove(Actor::MovableMeshActor* actor) {

            movableMeshOctree.Remove(actor, actor->aabb);

        }

        void SpacePartitioning::Add(Actor::StaticMeshActor* actor) {

            staticMeshOctree.Insert(actor, actor->aabb);

        }

        void SpacePartitioning::Remove(Actor::StaticMeshActor* actor) {

            staticMeshOctree.Remove(actor, actor->aabb);

        }



        void SpacePartitioning::Add(Lighting::Light* light) {

            lights.push_back(light);

        }

        void SpacePartitioning::Remove(Lighting::Light* light) {

            auto item = std::find(lights.begin(), lights.end(), light);

            if (item != lights.end()) {
                lights.erase(item);
            }

        }

        void SpacePartitioning::GetRenderList(Volume::Frustum frustum, RenderList& renderList) {

            std::vector<Actor::MovableMeshActor*> movableActors;
            std::vector<Actor::MovableMeshActor*> insideMovableActors;
            std::vector<Actor::StaticMeshActor*> staticActors;
            std::vector<Actor::StaticMeshActor*> insideStaticActors;

            movableMeshOctree.QueryFrustum(movableActors,
                insideMovableActors, frustum);
            staticMeshOctree.QueryFrustum(staticActors,
                insideStaticActors, frustum);

            for (auto actor : movableActors) {
                if (actor->dontCull || actor->visible && frustum.Intersects(actor->aabb))
                    renderList.Add(actor);
            }

            for (auto actor : insideMovableActors)
                if (actor->visible)
                    renderList.Add(actor);

            for (auto actor : staticActors) {
                if (actor->dontCull || actor->visible && frustum.Intersects(actor->aabb)) {
                    renderList.Add(actor);
                }
            }

            for (auto actor : insideStaticActors)
                if (actor->visible)
                    renderList.Add(actor);
        
        }

        std::vector<Actor::Actor*> SpacePartitioning::GetActors() {

            std::vector<Actor::Actor*> actors;

            auto meshActors = GetMeshActors();

            for (auto actor : meshActors)
                actors.push_back(actor);

            return actors;

        }

        std::vector<Actor::MeshActor*> SpacePartitioning::GetMeshActors() {

            std::vector<Actor::MovableMeshActor*> movableActors;
            movableMeshOctree.GetData(movableActors);

            std::vector<Actor::StaticMeshActor*> staticActors;
            staticMeshOctree.GetData(staticActors);

            std::vector<Actor::MeshActor*> actors;

            actors.reserve(movableActors.size() +
                staticActors.size());

            for (auto actor : movableActors)
                actors.push_back(actor);
            for (auto actor : staticActors)
                actors.push_back(actor);

            return actors;

        }

        std::vector<Actor::MovableMeshActor*> SpacePartitioning::GetMovableMeshActors(Volume::AABB aabb) {

            std::vector<Actor::MovableMeshActor*> actors;

            movableMeshOctree.QueryAABB(actors, aabb);

            return actors;

        }


        std::vector<Actor::StaticMeshActor*> SpacePartitioning::GetStaticMeshActors(Volume::AABB aabb) {

            std::vector<Actor::StaticMeshActor*> actors;

            staticMeshOctree.QueryAABB(actors, aabb);

            return actors;

        }

        std::vector<Lighting::Light*> SpacePartitioning::GetLights() {
            return lights;
        }

        void SpacePartitioning::Clear() {
            movableMeshOctree.Clear();
            staticMeshOctree.Clear();
            lights.clear();
        }

    }

}