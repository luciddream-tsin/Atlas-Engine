#include "OpaqueRenderer.h"

#include <mutex>

namespace Atlas {

	namespace Renderer {

		std::string OpaqueRenderer::vertexPath = "deferred/geometry.vsh";
		std::string OpaqueRenderer::fragmentPath = "deferred/geometry.fsh";

		Shader::ShaderBatch OpaqueRenderer::shaderBatch;
		std::mutex OpaqueRenderer::shaderBatchMutex;

		OpaqueRenderer::OpaqueRenderer() {

			renderList = RenderList(AE_OPAQUE_CONFIG);

			modelMatrixUniform = shaderBatch.GetUniform("mMatrix");
			viewMatrixUniform = shaderBatch.GetUniform("vMatrix");
			projectionMatrixUniform = shaderBatch.GetUniform("pMatrix");

			diffuseColorUniform = shaderBatch.GetUniform("diffuseColor");
			specularColorUniform = shaderBatch.GetUniform("specularColor");
			ambientColorUniform = shaderBatch.GetUniform("ambientColor");
			specularHardnessUniform = shaderBatch.GetUniform("specularHardness");
			specularIntensityUniform = shaderBatch.GetUniform("specularIntensity");

		}

		void OpaqueRenderer::Render(Viewport* viewport, RenderTarget* target, Camera* camera, Scene::Scene* scene) {

			std::lock_guard<std::mutex> guard(shaderBatchMutex);

			bool backFaceCulling = true;

			renderList.Clear();

			Volume::AABB base(vec3(-1.0f), vec3(1.0f));
			auto aabb = base.Transform(glm::inverse(camera->projectionMatrix * camera->viewMatrix));

			scene->GetRenderList(camera->frustum, aabb, renderList);

			for (auto& renderListBatchesKey : renderList.orderedRenderBatches) {

				auto shaderID = renderListBatchesKey.first;
				auto renderListBatches = renderListBatchesKey.second;

				shaderBatch.Bind(shaderID);

				viewMatrixUniform->SetValue(camera->viewMatrix);
				projectionMatrixUniform->SetValue(camera->projectionMatrix);

				for (auto renderListBatch : renderListBatches) {

					auto meshActorBatch = renderListBatch.meshActorBatch;

					// If there is no actor of that mesh visible we discard it.
					if (!meshActorBatch->GetSize()) {
						continue;
					}

					auto mesh = meshActorBatch->GetObject();
					mesh->Bind();

					if (!mesh->cullBackFaces && backFaceCulling) {
						glDisable(GL_CULL_FACE);
						backFaceCulling = false;
					}
					else if (mesh->cullBackFaces && !backFaceCulling) {
						glEnable(GL_CULL_FACE);
						backFaceCulling = true;
					}

					// Prepare uniform buffer here
					// Generate all drawing commands
					// We could also batch several materials together because they share the same shader

					// Render the sub data of the mesh that use this specific shader
					for (auto& subData : renderListBatch.subData) {

						auto material = subData->material;

						if (material->HasDiffuseMap())
							material->diffuseMap->Bind(GL_TEXTURE0);
						if (material->HasNormalMap())
							material->normalMap->Bind(GL_TEXTURE1);
						if (material->HasSpecularMap())
							material->specularMap->Bind(GL_TEXTURE2);
						if (material->HasDisplacementMap())
							material->displacementMap->Bind(GL_TEXTURE3);

						diffuseColorUniform->SetValue(material->diffuseColor);
						specularColorUniform->SetValue(material->specularColor);
						ambientColorUniform->SetValue(material->ambientColor);
						specularHardnessUniform->SetValue(material->specularHardness);
						specularIntensityUniform->SetValue(material->specularIntensity);

						// We could also use instanced rendering here
						for (auto& actor : meshActorBatch->actors) {

							modelMatrixUniform->SetValue(actor->transformedMatrix);

							glDrawElements(mesh->data.primitiveType, subData->indicesCount, mesh->data.indices.GetType(),
										   (void*)((uint64_t)(subData->indicesOffset * mesh->data.indices.GetElementSize())));

						}

					}

				}

			}

		}

		void OpaqueRenderer::InitShaderBatch() {

			std::lock_guard<std::mutex> guard(shaderBatchMutex);

			shaderBatch.AddStage(AE_VERTEX_STAGE, vertexPath);
			shaderBatch.AddStage(AE_FRAGMENT_STAGE, fragmentPath);

		}

		void OpaqueRenderer::AddConfig(Shader::ShaderConfig* config) {

			std::lock_guard<std::mutex> guard(shaderBatchMutex);

			shaderBatch.AddConfig(config);

		}

		void OpaqueRenderer::RemoveConfig(Shader::ShaderConfig* config) {

			std::lock_guard<std::mutex> guard(shaderBatchMutex);

			shaderBatch.RemoveConfig(config);

		}

	}

}