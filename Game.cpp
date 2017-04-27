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

	for (auto& e : entities) delete e;
	for (auto& m : meshes) delete m;
	delete camera;
	delete camera2;
	metalSRV->Release();
	normalSRV->Release();
	sampler1->Release();

	skySRV->Release();
	rsSky->Release();
	dsSky->Release();
	playButtonTexture->Release();
	quitButtonTexture->Release();
	titleTexture->Release();

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
	//CreateDDSTextureFromFile(device, L"Debug/TextureFiles/playpanel.dds", 0, &UITexture);
	CreateWICTextureFromFile(device, context, L"Debug/TextureFiles/cyanplaypanel.png",0, &playButtonTexture);
	CreateWICTextureFromFile(device, context, L"Debug/TextureFiles/cyanquitpanel.png", 0, &quitButtonTexture);

	//Import texture for game title
	CreateWICTextureFromFile(device, context, L"Debug/TextureFiles/asteroids.png", 0, &titleTexture);

	////Import texture for the background
	//CreateWICTextureFromFile(device, context, L"Debug/TextureFiles/background.png", 0, &backgroundTexture);

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

	skyVS = new SimpleVertexShader(device, context);
	if (!skyVS->LoadShaderFile(L"Debug/SkyVS.cso"))
		skyVS->LoadShaderFile(L"SkyVS.cso");

	skyPS = new SimplePixelShader(device, context);
	if (!skyPS->LoadShaderFile(L"Debug/SkyPS.cso"))
		skyPS->LoadShaderFile(L"SkyPS.cso");
}

void Game::CreateMaterials()
{
	CreateWICTextureFromFile(device, context, L"Debug/TextureFiles/asteroid.tif", 0, &metalSRV);
	CreateWICTextureFromFile(device, context, L"Debug/TextureFiles/asteroidNormalMap.tif", 0, &normalSRV);
	CreateDDSTextureFromFile(device, L"Debug/TextureFiles/Star.dds", 0, &skySRV);

	D3D11_SAMPLER_DESC sampleDesc = {};
	sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampleDesc.MaxLOD = D3D11_FLOAT32_MAX;


	device->CreateSamplerState(&sampleDesc, &sampler1);
	//device->CreateSamplerState(&sampleDesc2, &sampler1);

	material1 = new Material(pixelShader, vertexShader, metalSRV, normalSRV, sampler1);
	//material2 = new Material(pixelShader, vertexShader, carpetSRV, sampler1);

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

	camera2 = new Camera(0, 5, -15, false);
	camera2->UpdateProjectionMatrix((float) width / height);

	camera2->Rotate(0, 0);
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

	planeMesh = new Mesh("Debug/Models/cube.obj", device);
	meshes.push_back(planeMesh);
	planeEntity = new GameEntity(planeMesh, material1);
	entities.push_back(planeEntity);

	cubeMesh = new Mesh("Debug/Models/cube.obj", device);
	meshes.push_back(cubeMesh);

	cubeEntity = new GameEntity(cubeMesh, material1);
	entities.push_back(cubeEntity);

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
		// Update the camera
		camera->Update(deltaTime);
		camera2->Update(deltaTime);

		entities[0]->UpdateWorldMatrix();
		entities[1]->UpdateWorldMatrix();
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
		////Draw the sky
		//vertexBuffer = entities[2]->GetMesh()->GetVertexBuffer();
		//indexBuffer = entities[2]->GetMesh()->GetIndexBuffer();

		////Set the buffers in the input assembler
		//context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		//context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		////Set up the sky shaders
		//skyVS->SetMatrix4x4("view", camera->GetView());
		//skyVS->SetMatrix4x4("projection", camera->GetProjection());
		//skyVS->CopyAllBufferData();
		//skyVS->SetShader();

		//skyPS->SetShaderResourceView("Sky", skySRV);
		//skyPS->CopyAllBufferData();
		//skyPS->SetShader();

		//context->RSSetState(rsSky);
		//context->OMSetDepthStencilState(dsSky, 0);
		//context->DrawIndexed(entities[2]->GetMesh()->GetIndexCount(), 0, 0);

		//// Reset the render states we've changed
		//context->RSSetState(0);
		//context->OMSetDepthStencilState(0, 0);

		spriteBatch->Begin();

		//Draw title, play and quit sprites
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
		for (int i = 0; i <= 1; i++) {
			renderer.SetVertexBuffer(entities[i], vertexBuffer);
			renderer.SetIndexBuffer(entities[i], indexBuffer);
			renderer.SetVertexShader(vertexShader, entities[i], camera2);
			renderer.SetPixelShader(pixelShader, entities[i], camera2);
			context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
			context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
			// Finally do the actual drawing
			context->DrawIndexed(entities[i]->GetMesh()->GetIndexCount(), 0, 0);
		}
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

void Game::Print()
{
	printf("Hello World");
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