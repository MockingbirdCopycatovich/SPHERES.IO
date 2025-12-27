// Assessment1.cpp: A program using the TL-Engine

#include <TL-Engine.h>	// TL-Engine include file and namespace
using namespace tle;

enum class gameState { 
	Start,
	Playing,
	Paused,
	GameOver
};

void main()
{
	// Create a 3D engine (using TLX engine here) and open a window for it
	I3DEngine* myEngine = New3DEngine( kTLX );
	myEngine->StartWindowed();

	// Add default folder for meshes and other media
	myEngine->AddMediaFolder( "C:\\Users\\adm\\TL-Engine\\Media" );

	/**** Set up your scene here ****/
	IMesh* waterMesh = myEngine->LoadMesh("water.x");
	IModel* water = waterMesh->CreateModel(0.0f, -5.0f, 0.0f);

	IMesh* islandMesh = myEngine->LoadMesh("island.x");
	IModel* island = islandMesh->CreateModel(0.0f, -5.0f, 0.0f);


	IMesh* sphereMesh = myEngine->LoadMesh("spheremesh.x");
	IModel* playerSphere = sphereMesh->CreateModel(0.0f, 10.0f, 0.0f);


	IMesh* cubeMesh = myEngine->LoadMesh("minicube.x");

	IModel* cube1 = cubeMesh->CreateModel(-80.0f, 2.5f, 80.0f);
	IModel* cube2 = cubeMesh->CreateModel(80.0f, 2.5f, 80.0f);
	IModel* cube3 = cubeMesh->CreateModel(80.0f, 2.5f, -80.0f);
	IModel* cube4 = cubeMesh->CreateModel(-80.0f, 2.5f, -80.0f);

	IMesh* skyMesh = myEngine->LoadMesh("sky.x");
	IModel* sky = skyMesh->CreateModel(0.0f, -960.0f, 0.0f);

	ICamera* camera = myEngine->CreateCamera(kManual, 0.0f, 200.0f, 0.0f);
	camera->RotateX(90.0f);

	gameState currentState = gameState::Start;

	const float kPlayerSpeed = 60.0f;
	const float kRotationSpeed = 120.0f;

	// The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		// Draw the scene
		myEngine->DrawScene();
		float deltaTimer = myEngine->Timer();

		/**** Update your scene each frame here ****/

		if (myEngine->KeyHit(Key_Escape)) myEngine->Stop();

		switch (currentState)
		{
		case gameState::Start:
			if (myEngine->KeyHit(Key_Space)) currentState = gameState::Playing;

			break;
		case gameState::Playing:
		{
			float moveSpeed = deltaTimer * kPlayerSpeed;
			float rotationSpeed = deltaTimer * kRotationSpeed;

			if (myEngine->KeyHit(Key_P)) currentState = gameState::Paused;

			if (myEngine->KeyHeld(Key_W)) { 
				playerSphere->MoveLocalZ(moveSpeed);
			}
			if (myEngine->KeyHeld(Key_S)) { 
				playerSphere->MoveLocalZ(-moveSpeed);
			}
			if (myEngine->KeyHeld(Key_A)) { 
				playerSphere->RotateY(-rotationSpeed);
			}
			if (myEngine->KeyHeld(Key_D)) { 
				playerSphere->RotateY(rotationSpeed);
			}
			break;
		}
		case gameState::Paused:
			if (myEngine->KeyHit(Key_P)) currentState = gameState::Playing;
			break;
		case gameState::GameOver:
			break;
		default:
			break;
		}

	}

	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}
