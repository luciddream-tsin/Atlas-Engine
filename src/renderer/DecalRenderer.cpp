#include "DecalRenderer.h"
#include "helper/GeometryHelper.h"

string DecalRenderer::vertexPath = "deferred/decal.vsh";
string DecalRenderer::fragmentPath = "deferred/decal.fsh";

DecalRenderer::DecalRenderer() {

    vertexArray = GeometryHelper::GenerateCubeVertexArray();

    shader = new Shader();

    shader->AddComponent(VERTEX_SHADER, vertexPath);
    shader->AddComponent(FRAGMENT_SHADER, fragmentPath);

    shader->Compile();

    GetUniforms();

}

void DecalRenderer::Render(Window *window, RenderTarget *target, Camera *camera, Scene *scene, bool masterRenderer) {

    vertexArray->Bind();

    shader->Bind();

    decalTexture->SetValue(0);

    viewMatrix->SetValue(camera->viewMatrix);
    projectionMatrix->SetValue(camera->projectionMatrix);

    for (auto& decal : scene->decals) {

        modelMatrix->SetValue(decal->matrix);
        decal->texture->Bind(GL_TEXTURE0);

        glDrawArrays(GL_TRIANGLES, 0, 36);

    }

}

void DecalRenderer::GetUniforms() {

    decalTexture = shader->GetUniform("decalTexture");
    modelMatrix = shader->GetUniform("mMatrix");
    viewMatrix = shader->GetUniform("vMatrix");
    projectionMatrix = shader->GetUniform("pMatrix");

}