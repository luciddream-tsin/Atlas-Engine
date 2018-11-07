#include "GeometryRenderer.h"

GeometryRenderer::GeometryRenderer() {



}

void GeometryRenderer::Render(Window* window, RenderTarget* target, Camera* camera, Scene* scene, bool masterRenderer) {

	for (auto actorBatch : scene->actorBatches) {

		Mesh* mesh = actorBatch->GetMesh();		
		mesh->Bind();

		for (auto subData : mesh->data->subData) {

			Material* material = mesh->data->materials[subData->materialIndex];
			
			material->Bind(camera->viewMatrix, camera->projectionMatrix);
			Uniform* modelMatrixUniform = material->GetModelMatrixUniform();

			for (auto actor : actorBatch->actors) {

				if (!actor->render) {
					continue;
				}

				modelMatrixUniform->SetValue(actor->transformedMatrix);

				glDrawElements(mesh->data->primitiveType, subData->numIndices, mesh->data->indices->GetType(), 
					(void*)(subData->indicesOffset * mesh->data->indices->GetElementSize()));

			}

		}

	}

}