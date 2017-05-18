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
#include "Renderer.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "Emitter.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	struct asteroidObject
	{
		bool hit;
		btRigidBody* body;
		asteroidObject(btRigidBody* b) : body(b), hit(false) {}
	};

	struct bulletObject
	{
		bool hitbullet;
		btRigidBody* bulletBody;
		bulletObject(btRigidBody* b) : bulletBody(b), hitbullet(false) {}
	};

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	btRigidBody* CreateAsteroid(float rad, float x, float y, float z, float mass);
	btRigidBody* CreateBullets(float rad, float x, float y, float z, float mass);

	void AddBulletToWorld(int bulletNumber);
	void RemoveAsteriod(int astNumber);
	
	void AddAsteroidToWorld(int astNumber);
	void RecycleBullets(int bulletNumber);

	// Overridden mouse input helper methods
	void OnMouseDown (WPARAM buttonState, int x, int y);
	void OnMouseUp	 (WPARAM buttonState, int x, int y);
	void OnMouseMove (WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta,   int x, int y);
private:

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders(); 
	void CreateMaterials();
	void CreateMatrices();
	void CreateBasicGeometry();


	std::vector<Mesh*> meshes;
	std::vector<GameEntity*> entities;
	Camera* camera;
	Camera* camera2;
	// Buffers to hold actual geometry data
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;

	ID3D11ShaderResourceView* metalSRV;
	ID3D11ShaderResourceView* normalSRV;

	ID3D11SamplerState* sampler1;

	// The matrices to go from model space to screen space
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;
	POINT currentMousePos;
	POINT difference;

	Renderer renderer;

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

	// Particle stuff
	ID3D11ShaderResourceView* particleTexture;
	SimpleVertexShader* particleVS;
	SimplePixelShader* particlePS;
	ID3D11DepthStencilState* particleDepthState;
	ID3D11BlendState* particleBlendState;
	Emitter* emitter;

	//Minimap stuff
	ID3D11ShaderResourceView* redSRV;
	ID3D11ShaderResourceView* yellowSRV;
	Mesh* minimapPlayer;
	GameEntity* minimapPlayerEntity;

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

	std::vector<asteroidObject*> asteroids;
	std::vector<bulletObject*> bullets;
	std::vector<GameEntity*> astEntities;
	std::vector<GameEntity*> bulletEntities;

	int bNum = 0;
	int bulletTimer  = 2.0f;
	int asteroidCount = 0;
	int asteroidDeathCounter = 0;
	float testTimer = 0.0f;
	float addAsteroidTimer = 5.0f;
	float asteroidDeathTimer = 10.0f;
	float fireTimer = 3.0f;
	bool testbool = true;
	bool prevTab;
	bool fire = false;

	//UI Stuff
	std::unique_ptr<SpriteBatch> spriteBatch;
	ID3D11ShaderResourceView* playButtonTexture;
	ID3D11ShaderResourceView* scoreTexture;
	XMFLOAT2 playSpritePosition = XMFLOAT2(500,300);
	bool mouseAtPlay = false;
	ID3D11ShaderResourceView* titleTexture;
	ID3D11ShaderResourceView* quitButtonTexture;
	XMFLOAT2 quitSpritePosition = XMFLOAT2(525, 500);
	ID3D11ShaderResourceView* backgroundTexture;
	bool mouseAtQuit = false;

	//Game State Management
	enum GameStateManager
	{
		MainMenu,
		GamePlay,
		Exit
	};
	GameStateManager gameState;
};

