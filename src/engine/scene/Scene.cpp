#include "Scene.h"

namespace Atlas {

    namespace Scene {

        Scene::Scene(vec3 min, vec3 max, int32_t depth) : SceneNode() {

            AddToScene(this, &rootMeshMap);

        }

        Scene::~Scene() {



        }

        Scene& Scene::operator=(const Scene& that) {

            if (this != &that) {

                SceneNode::operator=(that);

                sky = that.sky;
                postProcessing = that.postProcessing;

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

            UpdateBindlessIndexMaps();


        }

        bool Scene::HasChanged() {

            return hasChanged;

        }

        void Scene::Clear() {

            sky = Lighting::Sky();
            postProcessing = PostProcessing::PostProcessing();

            SceneNode::Clear();

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



        void Scene::UpdateBindlessIndexMaps() {

            std::set<Ref<Texture::Texture2D>> textures;

            uint32_t textureIdx = 0;
            uint32_t bufferIdx = 0;

            textureToBindlessIdx.clear();
            meshIdToBindlessIdx.clear();

            auto meshes = GetMeshes();
            for (auto& mesh : meshes) {
                if (!mesh.IsLoaded()) continue;

                for (auto &material: mesh->data.materials) {
                    if (material->HasBaseColorMap())
                        textures.insert(material->baseColorMap);
                    if (material->HasOpacityMap())
                        textures.insert(material->opacityMap);
                    if (material->HasNormalMap())
                        textures.insert(material->normalMap);
                    if (material->HasRoughnessMap())
                        textures.insert(material->roughnessMap);
                    if (material->HasMetalnessMap())
                        textures.insert(material->metalnessMap);
                    if (material->HasAoMap())
                        textures.insert(material->aoMap);
                }

                // Not all meshes might have a bvh
                if (!mesh->IsBVHBuilt())
                    continue;

                meshIdToBindlessIdx[mesh.GetID()] = bufferIdx++;
            }

            for (const auto& texture : textures) {

                textureToBindlessIdx[texture] = textureIdx++;

            }

        }

    }

}