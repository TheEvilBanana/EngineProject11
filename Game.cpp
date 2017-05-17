#include "Game.h"
#include "Vertex.h"
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include <parallel_invoke.h>



// For the DirectX Math library
using namespace DirectX;

#define max(a,b) (((a) > (b)) ? (a):(b))
#define min(a,b) (((a) < (b)) ? (a):(b))

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore( 
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{
	// Initialize fields
	vertexBuffer = 0;
	indexBuffer = 0;
	vertexShader = 0;
	pixelShader = 0;

	
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.");
#endif
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	// Release any (and all!) DirectX objects
	// we've made in the Game class
	// Delete our simple shader objects, which
	// will clean up their own internal DirectX stuff
	delete vertexShader;
	delete pixelShader;
	delete material1;
	delete skyVS;
	delete skyPS;
	delete particleVS;
	delete particlePS;
	delete material2;

	for (auto& e : entities) delete e;
	for (auto& m : meshes) delete m;
	
	for (auto& ae : astEntities) delete ae;
	for (auto& b : bulletEntities) delete b;
	delete camera;
	delete camera2;

	delete minimapPlayer;
	delete minimapPlayerEntity;

	//Clean up normal map stuff
	metalSRV->Release();
	normalSRV->Release();
	sampler1->Release();

	redSRV->Release();
	yellowSRV->Release();

	//clean up Skybox stuff
	skySRV->Release();
	rsSky->Release();
	dsSky->Release();

	//Clean Up UI stuff
	playButtonTexture->Release();
	quitButtonTexture->Release();
	titleTexture->Release();
	scoreTexture->Release();
	backgroundTexture->Release();
	frameTexture->Release();


	for (int i = 0; i < asteroids.size(); i++)
	{
		world->removeCollisionObject(asteroids[i]);
		btMotionState* motionState = asteroids[i]->getMotionState();
		btCollisionShape* shape = asteroids[i]->getCollisionShape();
		delete asteroids[i];
		delete shape;
		delete motionState;
	}

	for (int i = 0; i < bullets.size(); i++)
	{
		world->removeCollisionObject(bullets[i]);
		btMotionState* motionState = bullets[i]->getMotionState();
		btCollisionShape* shape = bullets[i]->getCollisionShape();
		delete bullets[i];
		delete shape;
		delete motionState;
	}

	delete world;
	delete collisionConfig;
	delete dispatcher;
	delete broadphase;
	delete solver;
	delete planeBody;
	delete sphereBody;
	delete plane;
	delete planeMotion;
	delete sphere;
	delete sphereMotion;
	//for (auto& a : asteroids) delete a;

	//Clean up Particle Stuff
	particleTexture->Release();
	particleBlendState->Release();
	particleDepthState->Release();
	delete emitter;
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateMaterials();
	CreateMatrices();
	CreateBasicGeometry();

	//UI

	//Create SpriteBatch
	spriteBatch.reset(new SpriteBatch(context));

	//Import texture for loading
	CreateWICTextureFromFile(device, context, L"Debug/TextureFiles/cyanplaypanel.png",0, &playButtonTexture);
	CreateWICTextureFromFile(device, context, L"Debug/TextureFiles/cyanquitpanel.png", 0, &quitButtonTexture);
	CreateWICTextureFromFile(device, context, L"Debug/TextureFiles/scoreUIBg.png", 0, &scoreTexture);
	CreateWICTextureFromFile(device, context, L"Debug/TextureFiles/Frame.png", 0, &frameTexture);

	//Import texture for game title
	CreateWICTextureFromFile(device, context, L"Debug/TextureFiles/asteroids.png", 0, &titleTexture);

	//Import texture for the background
	CreateWICTextureFromFile(device, context, L"Debug/TextureFiles/Background.png", 0, &backgroundTexture);

	//Import particle texture
	CreateWICTextureFromFile(device, context, L"Debug/TextureFiles/Shock.jpg", 0, &particleTexture);

	// A depth state for the particles
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // Turns off depth writing
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	device->CreateDepthStencilState(&dsDesc, &particleDepthState);


	// Blend for particles (additive)
	D3D11_BLEND_DESC blend = {};
	blend.AlphaToCoverageEnable = false;
	blend.IndependentBlendEnable = false;
	blend.RenderTarget[0].BlendEnable = true;
	blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&blend, &particleBlendState);

	// Set up particles
	emitter = new Emitter(
		5000,							// Max particles
		100,							// Particles per second
		3,								// Particle lifetime
		1.0f,							// Start size
		1.0f,							// End size
		XMFLOAT4(1, 1.0f, 1.0f, 0.2f),	// Start color
		XMFLOAT4(1, 1.0f, 1.0f, 0.2f),		// End color
		XMFLOAT3(0, 5, 10),				// Start velocity
		XMFLOAT3(0, 5, 10),				// Start position
		XMFLOAT3(0, 0, 10),				// Start acceleration
		device,
		particleVS,
		particlePS,
		particleTexture
	);

	//set up directional lights
	dirLight1.SetLightValues(XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 0));
	dirLight2.SetLightValues(XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 0));
	
	/*********Trial Bullet*******/
	//btBoxShape* box = new btBoxShape(btVector3(1, 1, 1)); 
	collisionConfig = new btDefaultCollisionConfiguration();      // Setting the collision properties to default
	dispatcher = new btCollisionDispatcher(collisionConfig);      // Supports Algorithm that handle different collision types
	broadphase = new btDbvtBroadphase();                          // Collision detection algorithm i.e way of looping through objects to check collisions
	solver = new btSequentialImpulseConstraintSolver();           // Calculates everything i.e force, speed etc on collision

	world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfig);  // Create world using the above 4 properties
	world->setGravity(btVector3(0, 0, 0));                                                // Setting world gravity
	
	btTransform planeTransform;                                                         // Initiate transform
	planeTransform.setIdentity();                                                       // Setup transform
	planeTransform.setOrigin(btVector3(0, 0, 0));                                       // Set starting position
	plane = new btStaticPlaneShape(btVector3(0, 1, 0), 0);                              // Set the rigid body shape [plane], apply given transform
	planeMotion = new btDefaultMotionState(planeTransform);                             // Set up body motion, but this is static so no matter
	btRigidBody::btRigidBodyConstructionInfo infoPlane(0.0, planeMotion, plane);        // Set the info for the rigid body
	planeBody = new btRigidBody(infoPlane);                                             // Initiate the rigid body
	world->addRigidBody(planeBody);                                                     // Add body to world

	for (int i = 0; i < 5; i++)
	{
		CreateAsteroid(1, 5+(i * 2), 5, 5, 1);
	}

	/*for (int i = 0; i < 5; i++)
	{
		CreateBullets(1, 5, 5, 2, 1);
	}*/

	//asteroids[0]->setLinearVelocity(btVector3(-1, 0, 0));

	btTransform sphereTransform;                                                         // Same stuff for sphere as above
	sphereTransform.setIdentity();
	sphereTransform.setOrigin(btVector3(0, 10, 0));
	sphere = new btSphereShape(1);
	sphereMotion = new btDefaultMotionState(sphereTransform);
	btVector3 inertiaSphere;
	sphere->calculateLocalInertia(1.0, inertiaSphere);
	btRigidBody::btRigidBodyConstructionInfo infoSphere(1.0, sphereMotion, sphere, inertiaSphere);  // Setting inertia here cause its not static,i.e, dynamic cause it has mass
	sphereBody = new btRigidBody(infoSphere);
	world->addRigidBody(sphereBody);


	
	/************************************************************/
	
	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files using
// my SimpleShader wrapper for DirectX shader manipulation.
// - SimpleShader provides helpful methods for sending
//   data to individual variables on the GPU
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = new SimpleVertexShader(device, context);
	if (!vertexShader->LoadShaderFile(L"Debug/VertexShader.cso"))
		vertexShader->LoadShaderFile(L"VertexShader.cso");		

	pixelShader = new SimplePixelShader(device, context);
	if(!pixelShader->LoadShaderFile(L"Debug/PixelShader.cso"))	
		pixelShader->LoadShaderFile(L"PixelShader.cso");

	//Load Skybox vertex and pixel shaders
	skyVS = new SimpleVertexShader(device, context);
	if (!skyVS->LoadShaderFile(L"Debug/SkyVS.cso"))
		skyVS->LoadShaderFile(L"SkyVS.cso");

	skyPS = new SimplePixelShader(device, context);
	if (!skyPS->LoadShaderFile(L"Debug/SkyPS.cso"))
		skyPS->LoadShaderFile(L"SkyPS.cso");

	//load particle vertex and pixel shaders
	particleVS = new SimpleVertexShader(device, context);
	if (!particleVS->LoadShaderFile(L"Debug/ParticleVS.cso"))
		particleVS->LoadShaderFile(L"ParticleVS.cso");

	particlePS = new SimplePixelShader(device, context);
	if (!particlePS->LoadShaderFile(L"Debug/ParticlePS.cso"))
		particlePS->LoadShaderFile(L"ParticlePS.cso");
}

void Game::CreateMaterials()
{
	//import texture and normal map for asteroid
	CreateWICTextureFromFile(device, context, L"Debug/TextureFiles/asteroid.tif", 0, &metalSRV);
	CreateWICTextureFromFile(device, context, L"Debug/TextureFiles/asteroidNormalMap.tif", 0, &normalSRV);

	//import texture dds file for skybox
	CreateDDSTextureFromFile(device, L"Debug/TextureFiles/Sky.dds", 0, &skySRV);

	CreateWICTextureFromFile(device, context, L"Debug/TextureFiles/red.jpg", 0, &redSRV);
	CreateWICTextureFromFile(device, context, L"Debug/TextureFiles/yellow.jpg", 0, &yellowSRV);

	D3D11_SAMPLER_DESC sampleDesc = {};
	sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampleDesc.MaxLOD = D3D11_FLOAT32_MAX;


	device->CreateSamplerState(&sampleDesc, &sampler1);
	//device->CreateSamplerState(&sampleDesc2, &sampler1);

	material1 = new Material(pixelShader, vertexShader, metalSRV, normalSRV, sampler1);
	material2 = new Material(pixelShader, vertexShader, yellowSRV, normalSRV, sampler1);

	//Setting the sky stuff

	//Set up the rasterize state
	D3D11_RASTERIZER_DESC rsDesc = {};
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_FRONT;
	rsDesc.DepthClipEnable = true;
	device->CreateRasterizerState(&rsDesc, &rsSky);

	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&dsDesc, &dsSky);
}



// --------------------------------------------------------
// Initializes the matrices necessary to represent our geometry's 
// transformations and our 3D camera
// --------------------------------------------------------
void Game::CreateMatrices()
{
	camera = new Camera(0, 6, -15, true);
	camera->UpdateProjectionMatrix((float) width / height);

	camera2 = new Camera(0, 50, 0, false);
	camera2->UpdateProjectionMatrix((float) width / height);
	camera2->Rotate(1.5, 0);
}


// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	sphereMesh = new Mesh("Debug/Models/asteroid.obj", device);
	meshes.push_back(sphereMesh);

	sphereEntity = new GameEntity(sphereMesh, material1);
	entities.push_back(sphereEntity);

	for (int i = 0; i < 5; i++)
	{
		GameEntity* ast = new GameEntity(sphereMesh, material1);
		ast->SetScale(0.5, 0.5, 0.5);
		astEntities.push_back(ast);
	}

	/*for (int i = 0; i < 5; i++)
	{
		GameEntity* bul = new GameEntity(sphereMesh, material1);
		bul->SetScale(0.25, 0.25, 0.25);
		bulletEntities.push_back(bul);
	}*/

	//printf("ast size:" + astEntities.size());

	planeMesh = new Mesh("Debug/Models/cube.obj", device);
	meshes.push_back(planeMesh);
	planeEntity = new GameEntity(planeMesh, material1);
	entities.push_back(planeEntity);

	cubeMesh = new Mesh("Debug/Models/cube.obj", device);
	meshes.push_back(cubeMesh);

	cubeEntity = new GameEntity(cubeMesh, material1);
	entities.push_back(cubeEntity);

	minimapPlayer = new Mesh("Debug/Models/cube.obj", device);
	minimapPlayerEntity = new GameEntity(minimapPlayer, material2);

	entities[1]->SetScale(8.0f, 0.1f, 8.0f);
}




// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	// Update the projection matrix assuming the
	// camera exists
	if (camera)
		camera->UpdateProjectionMatrix((float) width / height);

	if (camera2)
		camera2->UpdateProjectionMatrix((float) width / height);
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	//Game State Management
	if (mouseAtPlay)
	{
		gameState = GamePlay;
		float sinTime = (sin(totalTime * 2) + 2.0f) / 10.0f;

		world->stepSimulation(deltaTime);
		btTransform sphereSpace;
		sphereBody->getMotionState()->getWorldTransform(sphereSpace);

		entities[0]->SetPosition(sphereSpace.getOrigin().x(), sphereSpace.getOrigin().y(), sphereSpace.getOrigin().z());
		
		addAsteroidTimer -= deltaTime;

		float xPosition = rand() % 1;
		if (rand() % 2 == 0)
		{
			xPosition = -xPosition;
		}
		float yPosition = rand() % 1;
		if (rand() % 2 == 0)
		{
			yPosition = -yPosition;
		}
		float zPosition = rand() % 1;
		if (rand() % 2 == 0)
		{
			zPosition = -zPosition;
		}

		if (addAsteroidTimer <= 0.0f)
		{
			GameEntity* ast = new GameEntity(sphereMesh, material1);
			ast->SetScale(0.5, 0.5, 0.5);
			astEntities.push_back(ast);

			addAsteroidTimer = 5.0f;
			CreateAsteroid(1, xPosition, yPosition, zPosition, 1.0);

			asteroidCount++;
		}

		fireTimer -= deltaTime;

		if (fireTimer <= 0.0f && (GetAsyncKeyState('F') & 0x8000))
		{
			GameEntity* bul = new GameEntity(sphereMesh, material1);
			bul->SetScale(0.25, 0.25, 0.25);
			bulletEntities.push_back(bul);
			printf("bullet created");
			fireTimer = 3.0f;
			CreateBullets(0.25, 3, 3, 3, 1.0);
		}

		for (int i = 0; i < astEntities.size(); i++)
		{
			btTransform astSpace;
			asteroids[i]->getMotionState()->getWorldTransform(astSpace);
			astEntities[i]->SetPosition(astSpace.getOrigin().x(), astSpace.getOrigin().y(), astSpace.getOrigin().z());

			astEntities[i]->UpdateWorldMatrix();
		}

		for (int i = 0; i < bulletEntities.size(); i++)
		{
			btTransform bulSpace;
			bullets[i]->getMotionState()->getWorldTransform(bulSpace);
			bulletEntities[i]->SetPosition(bulSpace.getOrigin().x(), bulSpace.getOrigin().y(), bulSpace.getOrigin().z());

			bulletEntities[i]->UpdateWorldMatrix();
		}
		
		XMFLOAT3 playerPos = camera->GetPosition();
		minimapPlayerEntity->SetPosition(playerPos.x, playerPos.y, playerPos.z);

		//5/16/2017
		asteroidDeathTimer -= deltaTime;

		if (asteroidDeathTimer <= 0.0f)
		{
			asteroidDeathTimer = 5.0f;
			if (asteroids[asteroidDeathCounter]->isInWorld())
			{
				RemoveAsteriod(asteroidDeathCounter);
				asteroidDeathCounter++;
			}
		}

		// Update the camera
		camera->Update(deltaTime);
		camera2->Update(deltaTime);

		entities[0]->UpdateWorldMatrix();
		entities[1]->UpdateWorldMatrix();
		minimapPlayerEntity->UpdateWorldMatrix();

		//Asteroid Movement
		sphereEntity->Move(5.0f, 0.0f, 0);
		sphereEntity->Rotate(0.001f, 0.001f, 0);

		//Trail particle
		static bool isTabPressedLastFrame = false;
		static float shootTimer = 0.0f;
		bool isTabPressed = GetAsyncKeyState(VK_TAB);
		

		bool currentTab = (GetAsyncKeyState('F') & 0x8000) != 0;
		if (currentTab && !prevTab)
		{
			fire = !fire;

			if (fire)
				bNum++;

			if (bNum >= 5)
				bNum = 0;
		}
			
		prevTab = currentTab;
		
		/*if (fire)
		{
			if (!(bullets[bNum]->isInWorld()))
				AddBulletToWorld(bNum);
		}*/

		
		if (isTabPressed && bulletTimer <= 0.0f)
		{
			
			bulletTimer = 2.0f;
		}

		if (!isTabPressedLastFrame && isTabPressed && shootTimer <= 0.0f)
		{
			shootTimer = 0.1f;
		}
		isTabPressedLastFrame = isTabPressed;

		if (shootTimer > 0.0f)
		{
			emitter->SpawnParticle();
			
			//AddBulletToWorld(0);
			//bNum++;
			shootTimer -= deltaTime;
		}
		emitter->UpdateEmitterPosition(deltaTime);
	
		emitter->Update(deltaTime);
		//emitter->UpdateEmitterVelocity();
	}
	else
	{
		gameState = MainMenu;
	}

	if (mouseAtQuit && gameState==MainMenu)
	{
		gameState = Exit;
	}
	
	
	/*tbb::parallel_invoke(
		[&]() { camera->Update(deltaTime); },
		[&]() { entities[0]->UpdateWorldMatrix(); },
		[]() {printf("Hello World"); }
	);*/
	
	

	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();
	
	

}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = {0.0f, 0.0f, 0.0f, 0.0f};

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(
		depthStencilView, 
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	context->RSSetViewports(1, &viewport);
	// Set buffers in the input assembler
	//  - Do this ONCE PER OBJECT you're drawing, since each object might
	//    have different geometry.

	switch (gameState)
	{
	case MainMenu:
		
		/************************************************************/
		//Main screen UI
		spriteBatch->Begin();

		//Draw title, play and quit sprites
		spriteBatch->Draw(backgroundTexture, XMFLOAT2(0,0));
		spriteBatch->Draw(titleTexture, XMFLOAT2(300, 100));
		spriteBatch->Draw(playButtonTexture, playSpritePosition);
		spriteBatch->Draw(quitButtonTexture, quitSpritePosition);
		
		spriteBatch->End();
		break;
	
	case GamePlay:
	{
		//Draw the sky
		vertexBuffer = entities[2]->GetMesh()->GetVertexBuffer();
		indexBuffer = entities[2]->GetMesh()->GetIndexBuffer();

		//Set the buffers in the input assembler
		context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		//Set up the sky shaders
		skyVS->SetMatrix4x4("view", camera->GetView());
		skyVS->SetMatrix4x4("projection", camera->GetProjection());
		skyVS->CopyAllBufferData();
		skyVS->SetShader();

		skyPS->SetShaderResourceView("Sky", skySRV);
		skyPS->CopyAllBufferData();
		skyPS->SetShader();

		context->RSSetState(rsSky);
		context->OMSetDepthStencilState(dsSky, 0);
		context->DrawIndexed(entities[2]->GetMesh()->GetIndexCount(), 0, 0);

		// Reset the render states we've changed
		context->RSSetState(0);
		context->OMSetDepthStencilState(0, 0);
		//*********************************************************//
		
		//Draw the actual asteroids objects.
		for (int i = 0; i <= 1; i++) {
			renderer.SetVertexBuffer(entities[i], vertexBuffer);
			renderer.SetIndexBuffer(entities[i], indexBuffer);
			renderer.SetVertexShader(vertexShader, entities[i], camera);
			renderer.SetPixelShader(pixelShader, entities[i], camera);
			context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
			context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
			// Finally do the actual drawing
			context->DrawIndexed(entities[i]->GetMesh()->GetIndexCount(), 0, 0);
		}
		//Asteroid spawning
		for (int i = 0; i < asteroids.size(); i++)
		{
			if (asteroids[i]->isInWorld())
			{
				renderer.SetVertexBuffer(astEntities[i], vertexBuffer);
				renderer.SetIndexBuffer(astEntities[i], indexBuffer);
				renderer.SetVertexShader(vertexShader, astEntities[i], camera);
				renderer.SetPixelShader(pixelShader, astEntities[i], camera);
				context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
				context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
				// Finally do the actual drawing
				context->DrawIndexed(astEntities[i]->GetMesh()->GetIndexCount(), 0, 0);
			}	
		}

		//Bullets
		for (int i = 0; i < bullets.size(); i++)
		{
			if (bullets[i]->isInWorld())
			{
				renderer.SetVertexBuffer(bulletEntities[i], vertexBuffer);
				renderer.SetIndexBuffer(bulletEntities[i], indexBuffer);
				renderer.SetVertexShader(vertexShader, bulletEntities[i], camera);
				renderer.SetPixelShader(pixelShader, bulletEntities[i], camera);
				context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
				context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
				// Finally do the actual drawing
				context->DrawIndexed(bulletEntities[i]->GetMesh()->GetIndexCount(), 0, 0);
			}
		}

		/***************************************************************/
		// Particle states`	


		float blend[4] = { 1,1,1,1 };
		context->OMSetBlendState(particleBlendState, blend, 0xffffffff);  // Additive blending
		context->OMSetDepthStencilState(particleDepthState, 0);			// No depth WRITING

																		// Draw the emitter
		emitter->Draw(context, camera);

		// Reset to default states for next frame
		context->OMSetBlendState(0, blend, 0xffffffff);
		context->OMSetDepthStencilState(0, 0);

		/*****************************************************************/

		//Score UI
		spriteBatch->Begin();
		spriteBatch->Draw(scoreTexture, XMFLOAT2(width / 2 - 600, height / 2 - 350));
		spriteBatch->End();


		/******************************************************************/
		//Mini Map
		spriteBatch->Begin();
		spriteBatch->Draw(frameTexture, XMFLOAT2(width / 2 + 500, height / 2 + 250));
		spriteBatch->End();
		const float color2[4] = {0.25f, 0.25f, 0.25f, 1.0f};
		// Clear the render target and depth buffer (erases what's on the screen)
		//  - Do this ONCE PER FRAME
		//  - At the beginning of Draw (before drawing *anything*)
		//context->ClearRenderTargetView(backBufferRTV, color2);
		context->ClearDepthStencilView(
			depthStencilView,
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			1.0f,
			0);
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		context->RSSetViewports(1, &viewportMiniMap);
		XMFLOAT3 entityPos;
		for (int i = 0; i <= 1; i++) {
			entityPos = entities[i]->GetPosition();
			renderer.SetVertexBuffer(entities[i], vertexBuffer);
			renderer.SetIndexBuffer(entities[i], indexBuffer);
			renderer.SetVertexShader(vertexShader, entities[i], camera2);
			renderer.SetPixelShaderMiniMap(pixelShader, entities[i], camera2, redSRV, entityPos, camera);
			context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
			context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
			// Finally do the actual drawing
			context->DrawIndexed(entities[i]->GetMesh()->GetIndexCount(), 0, 0);
		}

		//Asteroid spawning
		for (int i = 0; i < asteroids.size(); i++)
		{
			if (asteroids[i]->isInWorld())
			{
				entityPos = astEntities[i]->GetPosition();
				renderer.SetVertexBuffer(astEntities[i], vertexBuffer);
				renderer.SetIndexBuffer(astEntities[i], indexBuffer);
				renderer.SetVertexShader(vertexShader, astEntities[i], camera2);
				renderer.SetPixelShaderMiniMap(pixelShader, astEntities[i], camera2, redSRV, entityPos, camera);
				context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
				context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
				// Finally do the actual drawing
				context->DrawIndexed(astEntities[i]->GetMesh()->GetIndexCount(), 0, 0);
			}

		}

		entityPos = minimapPlayerEntity->GetPosition();
		renderer.SetVertexBuffer(minimapPlayerEntity, vertexBuffer);
		renderer.SetIndexBuffer(minimapPlayerEntity, indexBuffer);
		renderer.SetVertexShader(vertexShader, minimapPlayerEntity, camera2);
		renderer.SetPixelShader(pixelShader, minimapPlayerEntity, camera2);
		context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		// Finally do the actual drawing
		context->DrawIndexed(minimapPlayerEntity->GetMesh()->GetIndexCount(), 0, 0);

		}
		break;
	case Exit:
		Quit();
		break;
	default:
		break;
	}	

	swapChain->Present(0, 0);
	
}

btRigidBody* Game::CreateAsteroid(float rad, float x, float y, float z, float mass)
{
	btTransform sphereTransform;
	sphereTransform.setIdentity();
	sphereTransform.setOrigin(btVector3(x, y, z));
	btSphereShape* sphere = new btSphereShape(rad);
	btVector3 inertia(0, 0, 0);
	if (mass != 0.0f)
		sphere->calculateLocalInertia(mass, inertia);
	btMotionState* motion = new btDefaultMotionState(sphereTransform);
	btRigidBody::btRigidBodyConstructionInfo info(mass, motion, sphere, inertia);
	btRigidBody* body = new btRigidBody(info);
	world->addRigidBody(body);
	
	
	float xSpeed = rand() % 3;
	if (rand() % 2 == 0)
	{
		xSpeed = -xSpeed;
	}
	float ySpeed = rand() % 3;
	if (rand() % 2 == 0)
	{
		ySpeed = -ySpeed;
	}
	float zSpeed = rand() % 3;
	if (rand() % 2 == 0)
	{
		zSpeed = -zSpeed;
	}

	body->setLinearVelocity(btVector3(xSpeed, ySpeed, zSpeed));
	
	asteroids.push_back(body);

	/*GameEntity* ast = new GameEntity(sphereMesh, material1);
	ast->SetScale(0.5, 0.5, 0.5);
	astEntities.push_back(ast);*/

	return body;
}

btRigidBody * Game::CreateBullets(float rad, float x, float y, float z, float mass)
{
	btTransform sphereTransform;
	sphereTransform.setIdentity();
	sphereTransform.setOrigin(btVector3(x, y, z));
	btSphereShape* sphere = new btSphereShape(rad);
	btVector3 inertia(0, 0, 0);
	if (mass != 0.0f)
		sphere->calculateLocalInertia(mass, inertia);
	btMotionState* motion = new btDefaultMotionState(sphereTransform);
	btRigidBody::btRigidBodyConstructionInfo info(mass, motion, sphere, inertia);
	btRigidBody* body = new btRigidBody(info);
	world->addRigidBody(body);

	body->setLinearVelocity(btVector3(0, 0, 10));

	bullets.push_back(body);
	
	return body;
}

void Game::AddBulletToWorld(int bulletNumber)
{
	world->addRigidBody(bullets[bulletNumber]);
	bullets[bulletNumber]->setLinearVelocity(btVector3(0, 0, 10));
}

void Game::AddAsteroidToWorld(int astNumber)
{
	float x = rand() % 3;
	if (rand() % 2 == 0)
	{
		x = -x;
	}
	float y = rand() % 3;
	if (rand() % 2 == 0)
	{
		y = -y;
	}
	float z = rand() % 3;
	if (rand() % 2 == 0)
	{
		z = -z;
	}

	//world->addRigidBody(asteroids[astNumber]);
	asteroids[astNumber]->setLinearVelocity(btVector3(x, y, z));
}

void Game::RemoveAsteriod(int astNumber)
{
	world->removeRigidBody(asteroids[astNumber]);
	/*asteroids[astNumber]->setLinearVelocity(btVector3(0, 0, 0));
	

	btTransform astSpace;
	asteroids[astNumber]->getMotionState()->getWorldTransform(astSpace);
	astSpace.setOrigin(btVector3(0, 5, 0));
	asteroids[astNumber]->getMotionState()->setWorldTransform(astSpace);

	astEntities[astNumber]->SetPosition(astSpace.getOrigin().x(), astSpace.getOrigin().y(), astSpace.getOrigin().z());
	astEntities[astNumber]->UpdateWorldMatrix();*/
}

void Game::RecycleBullets(int bulletNumber)
{
	bullets[bulletNumber]->setLinearVelocity(btVector3(0, 0, 0));
	world->removeRigidBody(bullets[bulletNumber]);

	btTransform bulSpace;
	bullets[bulletNumber]->getMotionState()->getWorldTransform(bulSpace);
	bulSpace.setOrigin(btVector3(0, 3, -2));
	bullets[bulletNumber]->getMotionState()->setWorldTransform(bulSpace);

	bulletEntities[bulletNumber]->SetPosition(bulSpace.getOrigin().x(), bulSpace.getOrigin().y(), bulSpace.getOrigin().z());
	bulletEntities[bulletNumber]->UpdateWorldMatrix();
}




#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	if (((x > playSpritePosition.x) && (x < playSpritePosition.x + 250)) && ((y > playSpritePosition.y) && (y < playSpritePosition.y + 250)))
	{
		if (buttonState & 0x0001)
		{
			mouseAtPlay = true;
		}
	}
	if (((x > quitSpritePosition.x) && (x < quitSpritePosition.x + 250)) && ((y > quitSpritePosition.y) && (y < quitSpritePosition.y + 250)))
	{
		if (buttonState & 0x0001)
		{
			mouseAtQuit = true;
		}
	}
	// Caputure the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	SetCapture(hWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
// --------------------------------------------------------
void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
// --------------------------------------------------------
void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	// Check left mouse button
	if (buttonState & 0x0001) {
		float xDiff = (x - prevMousePos.x) * 0.005f;
		float yDiff = (y - prevMousePos.y) * 0.005f;
		camera->Rotate(yDiff, xDiff);
	}

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;
}

// --------------------------------------------------------
// Helper method for mouse wheel scrolling.  
// WheelDelta may be positive or negative, depending 
// on the direction of the scroll
// --------------------------------------------------------
void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	// Add any custom code here...
}
#pragma endregion