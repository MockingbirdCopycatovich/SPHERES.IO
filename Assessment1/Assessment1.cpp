// test.cpp: A program using the TL-Engine

#include <TL-Engine.h>	// TL-Engine include file and namespace
#include <cstdlib> 
#include <ctime> 

using namespace tle; 
using namespace std;

enum GameState
{
	Start,
	Playing,
	Paused,
	GameWon,
	GameOver
};

float random(float min, float max) { 
	return min + float((max - min + 1) * rand() / (RAND_MAX + 1.0)); 
}

bool Collision(IModel* a, IModel* b, float dist) {

	float dx = a->GetX() - b->GetX();
	float dz = a->GetZ() - b->GetZ();

	return dx * dx + dz * dz < dist * dist;
}

class Sphere {
protected:
	IModel* model;
	int points;
	float collisionDistance;
	const float kSphereSpeed = 60.0f;
	float speed;
	float scaleFactor;

	bool hyperState;
	float hyperTimer;
	float baseSpeed;
public:
	Sphere(IMesh* mesh, float x, float z, const string& skin = "") {
		model = mesh->CreateModel(x, 10, z);
		if (!skin.empty()) model->SetSkin(skin.c_str());
		points = 0;
		collisionDistance = 10.0f;
		speed = kSphereSpeed;
		baseSpeed = speed;
		scaleFactor = 1.2f;

		hyperState = false;
		hyperTimer = 0.0f;
	}

	IModel* getModel() { return model; }
	float getCollisionDistance() { return collisionDistance; }
	int getPoints() { return points; }

	void addPoints() {
		points += 10;
		if (points % 40 == 0) {
			model->Scale(scaleFactor);
			collisionDistance *= scaleFactor;
			model->SetY(10.0f);
		}
	}

	void activateHyper() {
		if (hyperState)return;
		hyperState = true;
		hyperTimer = 5.0f;
		speed = baseSpeed * 1.5;
		model->SetSkin("hypersphere.jpg");
	}

	void updateHyper(float dt) {
		if (hyperState) {
			hyperTimer -= dt;
			if (hyperTimer <= 0.0f) {
				hyperState = false;
				speed = baseSpeed;
			}
		}
	}

	bool isHyper() { return hyperState; }
};

class PlayerSphere : public Sphere {
	const float kRotationSpeed = 120.0f;
	float rotationSpeed;
public:
	PlayerSphere(IMesh* mesh) : Sphere(mesh, 0, 0) {
		rotationSpeed = kRotationSpeed;
	}
	void Control(I3DEngine* engine, float dt) {
		updateHyper(dt);
		if (engine->KeyHeld(Key_W)) model->MoveLocalZ(speed * dt);
		if (engine->KeyHeld(Key_S)) model->MoveLocalZ(-speed * dt);
		if (engine->KeyHeld(Key_A)) model->RotateY(-rotationSpeed * dt);
		if (engine->KeyHeld(Key_D)) model->RotateY(rotationSpeed * dt);
	}
};

class EnemySphere : public Sphere {
	float idleDirection;
public:
	EnemySphere(IMesh* mesh) : Sphere(mesh, 20, 20, "enemysphere.jpg") {
		idleDirection = random(0.0f, 360.0f);
		baseSpeed = baseSpeed * 3 / 4;
		speed = baseSpeed;
	}

	void update(IModel* target, float dt) {
		updateHyper(dt);

		if (!target) return;
		model->LookAt(target);
		model->MoveLocalZ(speed * dt);
	}
	void idle(float dt) {
		model->MoveLocalZ(speed * 0.5f * dt);

		float x = model->GetX();
		float z = model->GetZ();

		bool hitWall = false;

		if (x <= -95.0f || x >= 95.0f) hitWall = true;
		if (z <= -95.0f || z >= 95.0f) hitWall = true;

		if (hitWall) {
			idleDirection = random(90.0f, 270.0f);
			model->RotateY(idleDirection);
		}
	}
};

class CubeManager {
	static const int numCubes = 12;
	IModel* cubes[numCubes];
	IMesh* mesh;
	float collisionDistance;
public:
	CubeManager(IMesh* m) {
		mesh = m;
		collisionDistance = 2.5f;
		for (int i = 0; i < numCubes; i++)cubes[i] = nullptr;
	}

	void spawnAll(IModel* player, IModel* enemy) {
		for (int i = 0; i < numCubes; i++) {
			float x, z;
			do {
				x = random(-95.0, 95.0);
				z = random(-95.0, 95.0);
			} while ((abs(x - player->GetX()) < 15 && abs(z - player->GetZ()) < 15)
				|| (abs(x - enemy->GetX()) < 15 && abs(z - enemy->GetZ()) < 15));
			cubes[i] = mesh->CreateModel(x, 2.5f, z);
		}
	}
	IModel* getCube(int i) { return cubes[i]; }
	IModel* findClosest(IModel* seeker) {
		IModel* closest = nullptr;
		float leastDistance = FLT_MAX;
		for (int i = 0; i < numCubes; i++) {
			if (!cubes[i])continue;
			float dx = cubes[i]->GetX() - seeker->GetX();
			float dz = cubes[i]->GetZ() - seeker->GetZ();
			float d = sqrt(dx * dx + dz * dz);
			if (d < leastDistance) {
				leastDistance = d;
				closest = cubes[i];
			}
		}
		return closest;
	}
	void respawn(int i, IModel* player, IModel* enemy) {
		if (!cubes[i])return;
		float x, z;
		do {
			x = random(-95.0, 95.0);
			z = random(-95.0, 95.0);
		} while ((abs(x - player->GetX()) < 15 && abs(z - player->GetZ()) < 15)
			|| (abs(x - enemy->GetX()) < 15 && abs(z - enemy->GetZ()) < 15));
		cubes[i]->SetPosition(x, 2.5f, z);
	}
	float getCollisionDistance() { return collisionDistance; }
	
	void attractCubes(IModel* sphere, float dt) {
		for (int i = 0; i < numCubes; i++) {
			if (!cubes[i])continue;

			float dx = sphere->GetX() - cubes[i]->GetX();
			float dz = sphere->GetZ() - cubes[i]->GetZ();
			float dist = sqrt(dx * dx + dz * dz);

			if (dist <= 50.0f) cubes[i]->Move(dx * dt, 0, dz * dt);
		}
	}
};

class HyperCube {
	IModel* model;
	bool active;
	float collisionDistance;
public:
	HyperCube(IMesh* mesh, float x, float z) {
		model = mesh->CreateModel(x, 2.5f, z);
		model->SetSkin("hypercube.jpg");
		active = true;
		collisionDistance = 2.5;
	}
	IModel* getModel() { return model; }
	bool isActive() { return active; }

	void deActive() {
		active = false;
		model->SetPosition(0, -1000, 0);
	}
	float getCollisionDistance() { return collisionDistance; }
};

void main()
{
	// Create a 3D engine (using TLX engine here) and open a window for it
	I3DEngine* myEngine = New3DEngine(kTLX);
	myEngine->StartWindowed();

	// Add default folder for meshes and other media
	myEngine->AddMediaFolder("C:\\Users\\adm\\TL-Engine\\Media");

	/**** Set up your scene here ****/
	srand(time(NULL));

	IMesh* waterMesh = myEngine->LoadMesh("water.x");
	IModel* water = waterMesh->CreateModel(0.0f, -5.0f, 0.0f);

	IMesh* islandMesh = myEngine->LoadMesh("island.x");
	IModel* island = islandMesh->CreateModel(0.0f, -5.0f, 0.0f);

	IMesh* skyMesh = myEngine->LoadMesh("sky.x");
	IModel* sky = skyMesh->CreateModel(0.0f, -960.0f, 0.0f);

	IMesh* SphereMesh = myEngine->LoadMesh("spheremesh.x");
	IMesh* CubeMesh = myEngine->LoadMesh("minicube.x");

	PlayerSphere player(SphereMesh);
	EnemySphere enemy(SphereMesh);


	CubeManager cubes(CubeMesh);
	cubes.spawnAll(player.getModel(), enemy.getModel());
	float hx, hz;
	do {
		hx = random(-95.0f, 95.0f);
		hz = random(-95.0f, 95.0f);
	} while (
		(abs(hx - player.getModel()->GetX()) < 15 &&
		abs(hz - player.getModel()->GetZ()) < 15) ||
		(abs(hx - enemy.getModel()->GetX()) < 15 &&
			abs(hz - enemy.getModel()->GetZ()) < 15)
		);

	HyperCube hyper(CubeMesh, hx, hz);

	IFont* menuFont = myEngine->LoadFont("Arial", 36);
	IFont* titleFont = myEngine->LoadFont("Arial", 72);

	ICamera* camera = myEngine->CreateCamera(kManual, 0.0f, 200.0f, 0.0f);
	camera->RotateLocalX(90.0f);
	bool isIsometric = false;
	const float cameraSpeed = 100.0f;

	GameState state = Start;

	float deltaTime = 0.0f;

	// The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		// Draw the scene
		myEngine->DrawScene();
		/**** Update your scene each frame here ****/
		deltaTime = myEngine->Timer();

		if (myEngine->KeyHit(Key_Escape))myEngine->Stop();

		if (myEngine->KeyHit(Key_1)) {
			camera->SetPosition(0.0f, 200.0f, 0.0f);
			camera->ResetOrientation();
			camera->RotateLocalX(90.0f);
			isIsometric = false;
		}

		if (myEngine->KeyHit(Key_2)) {
			camera->SetPosition(150.0f, 150.0f, -150.0f);
			camera->ResetOrientation();
			camera->RotateX(45.0f);
			camera->RotateY(-45.0f);
			isIsometric = true;
		}

		int screenCenterX = 600;
		int screenCenterY = 300;

		switch (state)
		{
		case Start:
		{	
			titleFont->Draw("SPHERES.IO", screenCenterX - 200, screenCenterY, kWhite);
			menuFont->Draw("by Vladislav Vasilev", screenCenterX + 50, screenCenterY + 80, kLightGrey);
			menuFont->Draw("Press SPACE to Start", screenCenterX - 180, screenCenterY + 150, kRed);
			if (myEngine->KeyHit(Key_Space))state = Playing;
			break;
		}
		case Playing:
		{
			player.Control(myEngine, deltaTime);

			IModel* target = nullptr;
			if (hyper.isActive()) {
				target = hyper.getModel();
			}
			else {
				target = cubes.findClosest(enemy.getModel());
			}
			enemy.update(target, deltaTime);

			for (int i = 0; i < 12; i++) {
				IModel* cube = cubes.getCube(i);
				if (!cube)continue;

				if (Collision(player.getModel(), cube, player.getCollisionDistance() + cubes.getCollisionDistance())) {
					player.addPoints();
					cubes.respawn(i, player.getModel(), enemy.getModel());
				}

				if (Collision(enemy.getModel(), cube, enemy.getCollisionDistance() + cubes.getCollisionDistance())) {
					enemy.addPoints();
					cubes.respawn(i, player.getModel(), enemy.getModel());
				}

			}

			if (hyper.isActive()) {
				if (Collision(player.getModel(), hyper.getModel(), player.getCollisionDistance() + hyper.getCollisionDistance())) {
					player.activateHyper();
					hyper.deActive();
				}

				if (Collision(enemy.getModel(), hyper.getModel(), enemy.getCollisionDistance() + hyper.getCollisionDistance())) {
					enemy.activateHyper();
					hyper.deActive();
				}
			}

			if (!hyper.isActive()) {
				if (player.isHyper()) cubes.attractCubes(player.getModel(), deltaTime);
				else cubes.attractCubes(enemy.getModel(), deltaTime);
			}

			if (Collision(player.getModel(), enemy.getModel(), player.getCollisionDistance() + enemy.getCollisionDistance())) {
				int diff = abs(player.getPoints() - enemy.getPoints());

				if (diff <= 40) {
					player.getModel()->MoveLocalZ(-20 * deltaTime);
					enemy.getModel()->MoveLocalZ(-20 * deltaTime);
				}
				else {
					if (player.getPoints() > enemy.getPoints()) {
						for (int i = 0; i < 4;i++)player.addPoints();
						enemy.getModel()->SetPosition(0, -1000, 0);
						state = GameWon;
					}else{
						for (int i = 0; i < 4; i++)enemy.addPoints();
						player.getModel()->SetPosition(0, -1000, 0);
						state = GameOver;
						enemy.getModel()->RotateY(random(90.0f, 360.0f));
					}
				}
			}

			if (player.getPoints() >= 120) state = GameWon;
			if (enemy.getPoints() >= 120) {
				state = GameOver;
				enemy.getModel()->RotateY(random(90.0f, 360.0f));
			}

			if (!isIsometric) {
				float camMoveSpeed = cameraSpeed * deltaTime;
				if (myEngine->KeyHeld(Key_Up)) camera->MoveZ(camMoveSpeed);
				if (myEngine->KeyHeld(Key_Down)) camera->MoveZ(-camMoveSpeed);
				if (myEngine->KeyHeld(Key_Left)) camera->MoveX(-camMoveSpeed);
				if (myEngine->KeyHeld(Key_Right)) camera->MoveX(camMoveSpeed);
			}

			float playerPosX = player.getModel()->GetX();
			float playerPosZ = player.getModel()->GetZ();
			if (-100 >= playerPosX
				|| 100 <= playerPosX
				|| -100 >= playerPosZ
				|| 100 <= playerPosZ)
			{
				state = GameOver;
				enemy.getModel()->RotateY(random(90.0f, 360.0f));
			}

			menuFont->Draw(
				("Player: " + to_string(player.getPoints())).c_str(),
				900, 20, kWhite);

			menuFont->Draw(
				("Enemy: " + to_string(enemy.getPoints())).c_str(),
				900, 40, kRed);

			if (myEngine->KeyHit(Key_P))state = Paused;

			break;
		}
		case Paused:
		{
			menuFont->Draw(
				"Game is PAUSED", 500, 500, kWhite
			);
			if (myEngine->KeyHit(Key_P))state = Playing;
			break;
		}
		case GameOver:
		{
			titleFont->Draw("GAME OVER", screenCenterX - 100, screenCenterY - 50, kRed);

			menuFont->Draw(
				("Player: " + to_string(player.getPoints())).c_str(),
				screenCenterX - 50, screenCenterY + 100, kWhite
			);

			menuFont->Draw(
				("Enemy: " + to_string(enemy.getPoints())).c_str(),
				screenCenterX - 50, screenCenterY + 150, kWhite
			);

			enemy.idle(deltaTime);

			break;
		}
		case GameWon:
		{
			titleFont->Draw("YOU WON!", screenCenterX, screenCenterY, kGreen);
			menuFont->Draw(
				("Final Score: " + to_string(player.getPoints())).c_str(),
				420, 310, kWhite
			);

			menuFont->Draw("Press ESC to QUIT", 420, 420, kWhite);

			break;
		}
		default:
		{
			break;
		}
		}
	}

	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}
