#pragma once
#include <DirectXMath.h>

using namespace DirectX;

struct DirectionalLight {
	XMFLOAT4 ambientColor;
	XMFLOAT4 diffuseColor;
	XMFLOAT3 direction;

	void SetLightValues(XMFLOAT4 _ambientColor, XMFLOAT4 _diffuseColor, XMFLOAT3 _direction) {
		ambientColor = _ambientColor;
		diffuseColor = _diffuseColor;
		direction = _direction;
	}
};