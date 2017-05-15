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

XMFLOAT3 GameEntity::GetPosition()
{
	return position;
}





