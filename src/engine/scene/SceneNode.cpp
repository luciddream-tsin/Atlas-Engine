#include "SceneNode.h"
#include "Scene.h"

#include "../audio/AudioManager.h"

#include <algorithm>

namespace Atlas {

    namespace Scene {

        SceneNode::SceneNode(const SceneNode& that) {

            DeepCopy(that);

        }

        SceneNode& SceneNode::operator=(const SceneNode& that) {

            if (this != &that) {

                DeepCopy(that);

            }

            return *this;

        }

        void SceneNode::Add(SceneNode *node) {

            childNodes.push_back(node);

        }

        void SceneNode::Remove(SceneNode *node) {

            auto it = std::find(childNodes.begin(), childNodes.end(), node);

            if (it != childNodes.end()) {
                childNodes.erase(it);
            }

        }

        void SceneNode::Add(Actor::MovableMeshActor *actor) {

            movableMeshActors.push_back(actor);

            AddInternal(actor);

        }

        void SceneNode::Remove(Actor::MovableMeshActor *actor) {
            auto it = std::find(movableMeshActors.begin(), movableMeshActors.end(), actor);
            if (it != movableMeshActors.end()) {
                movableMeshActors.erase(it);
            }
            RemoveInternal(actor);
        }

        void SceneNode::Add(Actor::StaticMeshActor *actor) {
            AddInternal(actor);
        }

        void SceneNode::Remove(Actor::StaticMeshActor *actor) {


            auto it = std::find(staticMeshActors.begin(), staticMeshActors.end(), actor);

            if (it != staticMeshActors.end()) {
                staticMeshActors.erase(it);
            }

            RemoveInternal(actor);

        }




        void SceneNode::Add(Lighting::Light *light)  {



            lights.push_back(light);

        }

        void SceneNode::Remove(Lighting::Light *light) {



            auto it = std::find(lights.begin(), lights.end(), light);

            if (it != lights.end()) {
                lights.erase(it);
            }

        }



        void SceneNode::Clear() {

            childNodes.clear();

            movableMeshActors.clear();
            staticMeshActors.clear();
            lights.clear();

        }

        std::vector<Actor::MovableMeshActor*> SceneNode::GetNodeMovableMeshActors() {

            return movableMeshActors;

        }

        std::vector<Actor::StaticMeshActor*> SceneNode::GetNodeStaticMeshActors() {

            return staticMeshActors;

        }

        std::vector<Lighting::Light*> SceneNode::GetNodeLights() {

            return lights;

        }

        std::vector<SceneNode*> SceneNode::GetNodeChildren() {

            return childNodes;

        }


        bool SceneNode::Update(Camera* camera, float deltaTime, mat4 parentTransformation,
            bool parentTransformChanged) {

            bool changed = false;

            parentTransformChanged |= matrixChanged;

            changed |= parentTransformChanged;

            bool removed = false;

            if (matrixChanged) {
                globalMatrix = parentTransformation * matrix;
                matrixChanged = false;
            }

            for (auto &node : childNodes) {
                changed |= node->Update(camera, deltaTime, globalMatrix, parentTransformChanged);
            }

            // Only update the static mesh actors if the node moves (the static actors can't
            // be moved after being initialized by their constructor)
            if (parentTransformChanged) {

                for (auto &meshActor : staticMeshActors) {
                    meshActor->Update(*camera, deltaTime,
                        parentTransformation, true);
                }

            }


            for (auto &meshActor : movableMeshActors) {

                if (meshActor->HasMatrixChanged() || parentTransformChanged) {
                    removed = true;
                    changed = true;
                }

                meshActor->Update(*camera, deltaTime, 
                    globalMatrix, parentTransformChanged);

                if (removed) {
                    removed = false;
                }

            }
            for (auto &light : lights) {
                light->Update(camera);
            }

            return changed;
        }



        void SceneNode::DeepCopy(const SceneNode& that) {

            RemoveFromScene();

            matrixChanged = true;

            matrix = that.matrix;
            globalMatrix = that.globalMatrix;

            movableMeshActors = that.movableMeshActors;
            staticMeshActors = that.staticMeshActors;
            lights = that.lights;

            childNodes.resize(that.childNodes.size());

            for (size_t i = 0; i < that.childNodes.size(); i++)
                childNodes[i] = new SceneNode(*that.childNodes[i]);


        }

        void SceneNode::AddInternal(Actor::MeshActor* actor) {

            auto meshId = actor->mesh.GetID();
            auto it = meshMap->find(meshId);

            if (it == meshMap->end()) {
                (*meshMap)[meshId] = RegisteredMesh {
                    .mesh = actor->mesh,
                    .actorCount = 1
                };
            }
            else {
                it->second.actorCount++;
            }

        }

        void SceneNode::RemoveInternal(Actor::MeshActor* actor) {

            auto meshId = actor->mesh.GetID();
            auto it = meshMap->find(meshId);

            if (it == meshMap->end())
                return;

            it->second.actorCount--;

            if (!it->second.actorCount) {
                meshMap->erase(it->first);
            }

        }

    }

}