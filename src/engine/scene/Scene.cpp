#include "Scene.h"

namespace Atlas {

    namespace Scene {

        Scene::Scene(vec3 min, vec3 max, int32_t depth) : SceneNode(),
            SpacePartitioning(min, max, depth) {

            AddToScene(this, &rootMeshMap);

        }

        Scene::~Scene() {



        }

        Scene& Scene::operator=(const Scene& that) {

            if (this != &that) {

                SceneNode::operator=(that);
                SpacePartitioning::operator=(that);

                sky = that.sky;

                hasChanged = true;

            }

            return *this;

        }

        void Scene::Update(Camera *camera, float deltaTime) {

            auto meshes = GetMeshes();

            if (sky.sun) {
                sky.sun->Update(camera);
            }

            hasChanged = SceneNode::Update(camera, deltaTime, mat4(1.0f), false);

        }

        bool Scene::HasChanged() {

            return hasChanged;

        }

        void Scene::Clear() {

            sky = Lighting::Sky();

            SceneNode::Clear();
            SpacePartitioning::Clear();

        }

        std::vector<ResourceHandle<Mesh::Mesh>> Scene::GetMeshes() {

            std::vector<ResourceHandle<Mesh::Mesh>> meshes;

            // Not really efficient, but does the job
            for (auto& [meshId, registeredMesh] : rootMeshMap) {
                meshes.push_back(registeredMesh.mesh);
            }

            return meshes;

        }

        std::vector<Material*> Scene::GetMaterials() {

            std::vector<Material*> materials;


            auto meshes = GetMeshes();
            for (auto mesh : meshes) {
                if (!mesh.IsLoaded())
                    continue;
                for (auto& material : mesh->data.materials) {
                    materials.push_back(material.get());
                }
            }

            return materials;

        }


        void Scene::WaitForResourceLoad() {

            auto meshes = GetMeshes();

            for(auto mesh : meshes) {
                mesh.WaitForLoad();
            }

        }

        bool Scene::IsFullyLoaded() {

            bool loaded = true;

            auto meshes = GetMeshes();

            for(auto mesh : meshes) {
                loaded &= mesh.IsLoaded();
            }

            return loaded;

        }

    }

}