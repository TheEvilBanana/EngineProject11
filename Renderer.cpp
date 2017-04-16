#include "Renderer.h"

Renderer::Renderer() {
}


Renderer::~Renderer() {
}

void Renderer::SetLights() {
	dirLight1.SetLightValues(XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 0));
	dirLight2.SetLightValues(XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 0));
}

void Renderer::SetVertexBuffer(GameEntity* &gameEntity, ID3D11Buffer* &vertexBuffer) {
	vertexBuffer = gameEntity->GetMesh()->GetVertexBuffer();
}

void Renderer::SetIndexBuffer(GameEntity* &gameEntity, ID3D11Buffer* &indexBuffer) {
	indexBuffer = gameEntity->GetMesh()->GetIndexBuffer();
}

void Renderer::SetVertexShader(SimpleVertexShader* &vertexShader, GameEntity* &gameEntity, Camera* &camera) {
	vertexShader = gameEntity->GetMaterial()->GetVertexShader();
	vertexShader->SetMatrix4x4("world", *gameEntity->GetWorldMatrix());
	vertexShader->SetMatrix4x4("view", camera->GetView());
	vertexShader->SetMatrix4x4("projection", camera->GetProjection());

	vertexShader->CopyAllBufferData();
	vertexShader->SetShader();
}

void Renderer::SetPixelShader(SimplePixelShader* &pixelShader, GameEntity* &gameEntity, Camera* &camera) {
	SetLights();
	pixelShader = gameEntity->GetMaterial()->GetPixelShader();
	pixelShader->SetData("dirLight1", &dirLight1, sizeof(DirectionalLight));
	pixelShader->SetData("dirLight2", &dirLight2, sizeof(DirectionalLight));

	pixelShader->SetShaderResourceView("textureSRV", gameEntity->GetMaterial()->GetMaterialSRV());
	pixelShader->SetShaderResourceView("normalMapSRV", gameEntity->GetMaterial()->GetNormalSRV());
	pixelShader->SetSamplerState("basicSampler", gameEntity->GetMaterial()->GetMaterialSampler());

	pixelShader->CopyAllBufferData();
	pixelShader->SetShader();
}




