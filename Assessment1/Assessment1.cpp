// Assessment1.cpp: A program using the TL-Engine

#include <TL-Engine.h>	// TL-Engine include file and namespace
#include <cstdlib>
#include <ctime>

using namespace tle;
using namespace std;

enum gameState { 
	Start,
	Playing,
	Paused,
	GameOver
};

int random(int min, int max) {
	return min + int((max - min + 1) * rand() / (RAND_MAX + 1.0));
}

bool sphereCubeCollision(IModel* sphere, IModel* cube, float collisionDistance) {

	float dx = sphere->GetX() - cube->GetX();
	float dz = sphere->GetZ() - cube->GetZ();

	return collisionDistance * collisionDistance > dx * dx + dz * dz;
}

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

	const int numCubes = 12;
	IModel* cubes[numCubes];
	srand(time(NULL));

	for (int i = 0; i < numCubes; i++) {
		int x = random(-100, 100);
		int z = random(-100, 100);
		cubes[i] = cubeMesh->CreateModel(x, 2.5f, z);
	}

	int playerPoints = 0;
	float scaleFactor = 1.2f;
	float collisionDistance = 10.0f;

	IMesh* skyMesh = myEngine->LoadMesh("sky.x");
	IModel* sky = skyMesh->CreateModel(0.0f, -960.0f, 0.0f);

	ICamera* camera = myEngine->CreateCamera(kManual, 0.0f, 200.0f, 0.0f);
	camera->RotateX(90.0f);

	gameState currentState = gameState::Start;
	bool isIsoCamera = false;

	const float playerSpeed = 60.0f;
	const float rotationSpeed = 120.0f;
	const float cameraSpeed = 100.0f;

	IFont* gameFont = myEngine->LoadFont("Arial", 36);

	// The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		// Draw the scene
		myEngine->DrawScene();
		float deltaTimer = myEngine->Timer();

		/**** Update your scene each frame here ****/

		if (myEngine->KeyHit(Key_Escape)) myEngine->Stop();

		if (myEngine->KeyHit(Key_1)) {
			camera->SetPosition(0.0f, 200.0f, 0.0f);
			camera->ResetOrientation();
			camera->RotateX(90.0f);
			isIsoCamera = false;
		}

		if (myEngine->KeyHit(Key_2)) {
			camera->SetPosition(150.0f, 150.0f, -150.0f);
			camera->ResetOrientation();
			camera->RotateX(45.0f);
			camera->RotateY(-45.0f);
			isIsoCamera = true;
		}

		switch (currentState)
		{
		case gameState::Start:
		{
			gameFont->Draw("Press SPACE to START", 500, 300, kRed);
			if (myEngine->KeyHit(Key_Space))
				currentState = gameState::Playing;
			break;
		}
		case gameState::Playing:
		{
			float moveSpeed = deltaTimer * playerSpeed;
			float rotSpeed = deltaTimer * rotationSpeed;

			if (myEngine->KeyHit(Key_P)) currentState = gameState::Paused;

			if (myEngine->KeyHeld(Key_W)) { 
				playerSphere->MoveLocalZ(moveSpeed);
			}
			if (myEngine->KeyHeld(Key_S)) { 
				playerSphere->MoveLocalZ(-moveSpeed);
			}
			if (myEngine->KeyHeld(Key_A)) { 
				playerSphere->RotateY(-rotSpeed);
			}
			if (myEngine->KeyHeld(Key_D)) { 
				playerSphere->RotateY(rotSpeed);
			}

			if (!isIsoCamera) {
				float camMoveSpeed = cameraSpeed * deltaTimer;
				if (myEngine->KeyHeld(Key_Up)) camera->MoveZ(camMoveSpeed);
				if (myEngine->KeyHeld(Key_Down)) camera->MoveZ(-camMoveSpeed);
				if (myEngine->KeyHeld(Key_Left)) camera->MoveX(-camMoveSpeed);
				if (myEngine->KeyHeld(Key_Right)) camera->MoveX(camMoveSpeed);
			}
			float playerPosX = playerSphere->GetX();
			float playerPosZ = playerSphere->GetZ();
			if (-100 >= playerPosX 
				|| 100 <= playerPosX 
				|| -100 >= playerPosZ 
				|| 100 <= playerPosZ)
				currentState = gameState::GameOver;

			for (int i = 0; i < numCubes; i++) {
				if (cubes[i] != nullptr && sphereCubeCollision(playerSphere, cubes[i], collisionDistance)) {
					playerPoints += 10;

					cubes[i] = nullptr;

					if (playerPoints % 40 == 0 && playerPoints != 0) {
						playerSphere->Scale(scaleFactor);
						playerSphere->SetY(playerSphere->GetY() * scaleFactor);
					}
				}
			}
			gameFont->Draw("Score: " + to_string(playerPoints), 20, 20, kWhite);
			break;
		}
		case gameState::Paused:
		{
			if (myEngine->KeyHit(Key_P)) currentState = gameState::Playing;
			break;
		}
		case gameState::GameOver:
		{
			gameFont->Draw("GAME OVER", 500, 300, kRed);
			gameFont->Draw("Press R to RESTART", 430, 350, kRed);
			gameFont->Draw("Press ESC to QUIT", 460, 400, kRed);
			break;
		}
		default:
			break;
		}

	}

	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}
