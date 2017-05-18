#pragma once
#include <DirectXMath.h>


class Camera {
public:
	Camera(float x, float y, float z, bool cameraMove);
	~Camera();

	// Transformations
	void MoveRelative(float x, float y, float z);
	void MoveAbsolute(float x, float y, float z);
	void Rotate(float x, float y);

	// Updating
	void Update(float dt);
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float aspectRatio);

	// Getters
	DirectX::XMFLOAT3 GetPosition() { return position; }
	DirectX::XMFLOAT4 GetRotation() { return rotation; }
	DirectX::XMFLOAT4X4 GetView() { return viewMatrix; }
	DirectX::XMFLOAT4X4 GetProjection() { return projMatrix; }
	DirectX::XMFLOAT3 cameraDirection;
	

private:
	// Camera matrices
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projMatrix;

	// Transformations
	DirectX::XMFLOAT3 startPosition;
	DirectX::XMFLOAT4 rotation;
	DirectX::XMFLOAT3 position;
	
	float xRotation;
	float yRotation;

	bool cameraMove;
};
