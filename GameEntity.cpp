#include "GameEntity.h"



GameEntity::GameEntity(Mesh *entityMesh, Material *entityMaterial) {
	// Save the mesh
	this->mesh = entityMesh;
	this->material = entityMaterial;

	// Set up transform
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	position = XMFLOAT3(0, 0, 0);
	rotation = XMFLOAT3(0, 0, 0);
	scale = XMFLOAT3(1, 1, 1);
}


GameEntity::~GameEntity() {
	//delete entityMesh;
}

void GameEntity::UpdateWorldMatrix() {
	XMMATRIX trans = XMMatrixTranslation(position.x, position.y, position.z);
	XMMATRIX rotX = XMMatrixRotationX(rotation.x);
	XMMATRIX rotY = XMMatrixRotationY(rotation.y);
	XMMATRIX rotZ = XMMatrixRotationZ(rotation.z);
	XMMATRIX sc = XMMatrixScaling(scale.x, scale.y, scale.z);

	XMMATRIX total = sc * rotZ * rotY * rotX * trans;
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(total));
}

//void GameEntity::PrepareMaterial(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix) {
//	//XMStoreFloat4x4(&viewMatrix, _viewMatrix);
//	//XMStoreFloat4x4(&projectionMatrix, _projectionMatrix);
//	
//	vertexShader->SetMatrix4x4("view", viewMatrix);
//	vertexShader->SetMatrix4x4("projection", projectionMatrix);
//	
//	//UINT stride = sizeof(Vertex);
//	//UINT offset = 0;
//	pixelShader->SetShaderResourceView("textureSRV", entitySRV);
//	pixelShader->SetSamplerState("basicSampler", entitySampler);
//
//	pixelShader->CopyAllBufferData();
//	vertexShader->SetMatrix4x4("world", worldMatrix);
//	vertexShader->CopyAllBufferData();
//
//	vertexShader->SetShader();
//	pixelShader->SetShader();
//}





