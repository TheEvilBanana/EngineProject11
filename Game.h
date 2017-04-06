#pragma once

#include "DXCore.h"
#include "SimpleShader.h"
#include <DirectXMath.h>
#include "Mesh.h"
#include "GameEntity.h"
#include "Camera.h"
#include "Lights.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include <vector>


class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void Print();

	// Overridden mouse input helper methods
	void OnMouseDown (WPARAM buttonState, int x, int y);
	void OnMouseUp	 (WPARAM buttonState, int x, int y);
	void OnMouseMove (WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta,   int x, int y);
private:

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders(); 
	void CreateMatrices();
	void CreateBasicGeometry();


	std::vector<Mesh*> meshes;
	std::vector<GameEntity*> entities;
	Camera* camera;

	// Buffers to hold actual geometry data
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;

	ID3D11ShaderResourceView* flamesSRV;
	ID3D11ShaderResourceView* carpetSRV;
	ID3D11SamplerState* sampler1;
	ID3D11SamplerState* sampler2;

	// The matrices to go from model space to screen space
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;
	POINT currentMousePos;
	POINT difference;

	
	Mesh *entityMesh1;
	Mesh *entityMesh2;
	Mesh* sphereMesh;
	Mesh* cubeMesh;
	Mesh* planeMesh;

	Material *material1;
	Material *material2;
	
	GameEntity *entity1;
	GameEntity *entity2;
	GameEntity* sphereEntity;
	GameEntity* cubeEntity;
	GameEntity* planeEntity;

	DirectionalLight dirLight1;
	DirectionalLight dirLight2;

	//Sky stuff
	ID3D11ShaderResourceView* skySRV;
	SimpleVertexShader* skyVS;
	SimplePixelShader* skyPS;
	ID3D11RasterizerState* rsSky;
	ID3D11DepthStencilState* dsSky;

	
	

	//bulletstuff
	btDynamicsWorld* world;
	btDispatcher* dispatcher;
	btCollisionConfiguration* collisionConfig;
	btBroadphaseInterface* broadphase;
	btConstraintSolver* solver;
	btRigidBody* planeBody;
	btRigidBody* sphereBody;
	btStaticPlaneShape* plane;
	btMotionState* planeMotion;
	btSphereShape* sphere;
	btMotionState* sphereMotion;


};

