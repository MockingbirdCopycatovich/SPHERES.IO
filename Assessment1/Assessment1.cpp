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
	float speed;
	float scaleFactor;
public:
	Sphere(IMesh* mesh, float x, float z, const string& skin = "") {
		model = mesh->CreateModel(x, 10, z);
		if (!skin.empty()) model->SetSkin(skin.c_str());
		points = 0;
		collisionDistance = 10.0f;
		speed = 60.0f;
		scaleFactor = 1.2f;
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
};

class PlayerSphere : public Sphere {
	float rotationSpeed;
public:
	PlayerSphere(IMesh* mesh) : Sphere(mesh, 0, 0) {
		rotationSpeed = 120.0f;
	}
	void Control(I3DEngine* engine, float dt) {
		if (engine->KeyHeld(Key_W)) model->MoveLocalZ(speed * dt);
		if (engine->KeyHeld(Key_S)) model->MoveLocalZ(-speed * dt);
		if (engine->KeyHeld(Key_A)) model->RotateY(-rotationSpeed * dt);
		if (engine->KeyHeld(Key_D)) model->RotateY(rotationSpeed * dt);
	}
};

class EnemySphere : public Sphere {
public:
	EnemySphere(IMesh* mesh) : Sphere(mesh, 20, 20, "enemysphere.jpg"){}
	void update(IModel* target, float dt) {
		if (!target) return;
		model->LookAt(target);
		model->MoveLocalZ(speed * dt);
	}
};

class CubeManager {
	static const int numCubes = 12;
	IModel* cubes[numCubes];
	IMesh* mesh;
public:
	CubeManager(IMesh* m) {
		mesh = m;
		for (int i = 0; i < numCubes; i++)cubes[i] = nullptr;
	}

	void spawnALL(IModel* player, IModel* enemy, int i) {
		for (int i = 0; i < numCubes; i++) {
			float x, z;
			do {
				x = random(-95.0, 95.0);
				z = random(-95.0, 95.0);
			} while (abs(x - player->GetX()) < 15 && abs(z - player->GetZ()) < 15
				&& abs(x - enemy->GetX()) < 15 && abs(z - enemy->GetZ()) < 15);
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
		} while (abs(x - player->GetX()) < 15 && abs(z - player->GetZ()) < 15
			&& abs(x - enemy->GetX()) < 15 && abs(z - enemy->GetZ()) < 15);
		cubes[i] = mesh->CreateModel(x, 2.5f, z);
	}
};

void main()
{
	// Create a 3D engine (using TLX engine here) and open a window for it
	I3DEngine* myEngine = New3DEngine(kTLX);
	myEngine->StartWindowed();

	// Add default folder for meshes and other media
	myEngine->AddMediaFolder("C:\\Users\\adm\\TL-Engine\\Media");

	/**** Set up your scene here ****/


	// The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		// Draw the scene
		myEngine->DrawScene();

		/**** Update your scene each frame here ****/

	}

	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}
