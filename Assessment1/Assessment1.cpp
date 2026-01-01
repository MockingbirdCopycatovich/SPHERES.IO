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

float distanceXZ(float x1, float z1, float x2, float z2) {

	float dx = x1 - x2;
	float dz = z1 - z2;

	return sqrt(dx * dx + dz * dz);
}

void spawnCubes(IModel* cubes[], int numCubes, IMesh* cubeMesh, IModel* playerSphere) {
	const float spawnRange = 90.0f;
	const float cubeY = 2.5f;
	const float minCubeDistance = 10.0f;
	const float minPlayerDistance = 15.0f;

	for (int i = 0; i < numCubes; i++) {
		float x, z;
		bool validPosition;
		do {
			validPosition = true;
			x = random(-90, 90);
			z = random(-90, 90);

			if (distanceXZ(x, z, playerSphere->GetX(), playerSphere->GetZ()) < minPlayerDistance) {
				validPosition = false;
			}

			for (int j = 0; j < numCubes; j++) {
				if (cubes[j] != nullptr && distanceXZ(x, z, cubes[j]->GetX(), cubes[j]->GetZ()) < minCubeDistance) {
					validPosition = false;
				}
			}

		} while (!validPosition);

		if (cubes[i] == nullptr) {
			cubes[i] = cubeMesh->CreateModel(x, cubeY, z);
		}
		else {
			cubes[i]->SetPosition(x, cubeY, z);
		}
	}
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

	for (int i = 0; i < numCubes; i++) cubes[i] = nullptr;
	spawnCubes(cubes, numCubes, cubeMesh, playerSphere);

	bool hyperActive = false;
	float hyperTimer = 0.0f;
	const float hyperDuration = 5.0f;
	const float attractRadius = 50.0f;
	const float attractSpeed = 50.0f;

	IMesh* hyperCubeMesh = myEngine->LoadMesh("minicube.x");
	IModel* hyperCube = hyperCubeMesh->CreateModel(random(-80, 80), 2.5f, random(-80, 80));
	hyperCube->SetSkin("hypercube.jpg");

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

					cubes[i]->SetPosition(0.0f, -1000.0f, 0.0f);
					cubes[i] = nullptr;

					if (playerPoints % 40 == 0 && playerPoints != 0) {
						playerSphere->Scale(scaleFactor);
						playerSphere->SetY(playerSphere->GetY() * scaleFactor);
						collisionDistance *= scaleFactor;
					}
				}
				if (cubes[i] != nullptr && hyperActive) {
					float dx = playerSphere->GetX() - cubes[i]->GetX();
					float dz = playerSphere->GetZ() - cubes[i]->GetZ();
					float distance = sqrt(dx * dx + dz * dz);

					if (distance < attractRadius && distance > collisionDistance) {
						float moveStep = attractSpeed * deltaTimer;
						cubes[i]->MoveX((dx / distance) * moveStep);
						cubes[i]->MoveZ((dz / distance) * moveStep);
					}
				}
			}
			gameFont->Draw("Score: " + to_string(playerPoints), 20, 20, kWhite);

			if (playerPoints >= 120) {
				currentState = gameState::GameOver;
			}

			if (hyperCube != nullptr && sphereCubeCollision(playerSphere, hyperCube, collisionDistance)) {
				hyperActive = true;
				hyperTimer = hyperDuration;
				playerSphere->SetSkin("hypersphere.jpg");

				hyperCube->SetPosition(0.0f, -1000.0f, 0.0f);
				hyperCube = nullptr;
			}

			if (hyperActive) {
				hyperTimer -= deltaTimer;

				if (hyperTimer <= 0.0f) {
					hyperActive = false;
					
					float x = playerSphere->GetX();
					float y = playerSphere->GetY();
					float z = playerSphere->GetZ();

					playerSphere->~IModel();

					playerSphere = sphereMesh->CreateModel(x, y, z);
					
					playerSphere->Scale(pow(scaleFactor, playerPoints/40));
				}
			}

			break;
		}
		case gameState::Paused:
		{
			if (myEngine->KeyHit(Key_P)) currentState = gameState::Playing;
			break;
		}
		case gameState::GameOver:
		{
			if (playerPoints >= 120) {
				gameFont->Draw("CONGRATULATIONS!", 420, 300, kGreen);
				gameFont->Draw("You collected all cubes", 390, 350, kWhite);
				gameFont->Draw("Press ESC to Quit", 500, 400, kWhite);
				gameFont->Draw("Press R to RESTART", 550, 450, kWhite);

				if (myEngine->KeyHit(Key_R)) {
					playerPoints = 0;
					currentState = Playing;
					spawnCubes(cubes, numCubes, cubeMesh, playerSphere);
				}
			}
			else {
				gameFont->Draw("GAME OVER", 500, 300, kRed);
				gameFont->Draw("Press R to RESTART", 430, 350, kRed);
				gameFont->Draw("Press ESC to QUIT", 460, 400, kRed);
			}
			break;
		}
		default:
			break;
		}

	}

	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}
