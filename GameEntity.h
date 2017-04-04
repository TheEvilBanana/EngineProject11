#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "Mesh.h"
#include "Material.h"
#include "SimpleShader.h"

using namespace DirectX;

class GameEntity {
public:
	GameEntity(Mesh *entityMesh, Material *entityMaterial);
	~GameEntity();
	/*ID3D11Buffer* GetEntityVertexBuffer();
	ID3D11Buffer* GetEntityIndexBuffer();
	int GetEntityIndexCount();
	XMFLOAT4X4 worldMatrix;
	void setEntityPosition(float x, float y, float z);
	void setEntityRotate(float rotate);
	void setEntityScale(float x, float y, float z);
	void animate();
	void PrepareMaterial(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix);
	void SetPixelShader(SimplePixelShader *_pixelShader);
	void SetVertexShader(SimpleVertexShader *_vertexShader);
	void SetEntitySRV(ID3D11ShaderResourceView* _entitySRV);
	void SetEntitySampler(ID3D11SamplerState* _entitySampler);*/
	/*SimplePixelShader* pixelShader;
	SimpleVertexShader* vertexShader;*/

	void UpdateWorldMatrix();

	void Move(float x, float y, float z) { position.x += x;	position.y += y;	position.z += z; }
	void Rotate(float x, float y, float z) { rotation.x += x;	rotation.y += y;	rotation.z += z; }

	void SetPosition(float x, float y, float z) { position.x = x;	position.y = y;		position.z = z; }
	void SetRotation(float x, float y, float z) { rotation.x = x;	rotation.y = y;		rotation.z = z; }
	void SetScale(float x, float y, float z) { scale.x = x;		scale.y = y;		scale.z = z; }

	Mesh* GetMesh() { return mesh; }
	Material* GetMaterial() { return material; }
	DirectX::XMFLOAT4X4* GetWorldMatrix() { return &worldMatrix; }

private:
	
	/*XMFLOAT4X4 entityPosition;
	XMFLOAT4X4 entityRotation;
	XMFLOAT4X4 entityScale;
	ID3D11Buffer *entityVertexBufferMesh;
	ID3D11Buffer *entityIndexBufferMesh;
	int entityIndices1;
	SimplePixelShader* pixelShader;
	SimpleVertexShader* vertexShader;
	ID3D11ShaderResourceView* entitySRV;
	ID3D11SamplerState* entitySampler;*/
	//XMFLOAT4X4 viewMatrix;
	//XMFLOAT4X4 projectionMatrix;

	Mesh* mesh;
	Material* material;

	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT3 scale;

};

