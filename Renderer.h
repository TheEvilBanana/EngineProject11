#pragma once

#include "GameEntity.h"
#include "Camera.h"
#include "Lights.h"

class Renderer {
public:
	
	Renderer();
	~Renderer();

	void SetLights();

	void SetVertexBuffer(GameEntity* &gameEntity, ID3D11Buffer* &vertexBuffer);
	void SetIndexBuffer(GameEntity* &gameEntity, ID3D11Buffer* &indexBuffer);
	void SetVertexShader(SimpleVertexShader* &vertexShader, GameEntity* &gameEntity, Camera* &camera);
	void SetPixelShader(SimplePixelShader* &pixelShader, GameEntity* &gameEntity, Camera* &camera);

private:
	
	ID3D11Buffer *vertexBufferRender;
	ID3D11Buffer *indexBufferRender;
	SimpleVertexShader* vertexShaderRender;
	SimplePixelShader* pixelShaderRender;
	DirectionalLight dirLight1;
	DirectionalLight dirLight2;
	
};

