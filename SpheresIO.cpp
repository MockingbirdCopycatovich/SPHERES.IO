// CO1301 Games Concepts - Assessment 1
// Student Name: Vladislav Vasilev
// Student ID: G21303193
// Description: Spheres.io inspired 3D game using TL-Engine

#include <TL-Engine.h>	// TL-Engine include file and namespace
#include <cstdlib> 
#include <ctime> 

using namespace tle; 
using namespace std;

// Enumeration to control the different states of the game
enum GameState
{
	Start,      // Main menu
	Playing,    // Normal gameplay
	Paused,     // Game paused
	GameWon,    // Player wins
	GameOver    // Player loses
};

float random(float min, float max) { 
	return min + float((max - min + 1) * rand() / (RAND_MAX + 1.0)); 
}

// Checks collision between two models using distance-based detection
// Used for sphere-to-sphere and sphere-to-cube collision
bool Collision(IModel* a, IModel* b, float dist) {

	float dx = a->GetX() - b->GetX();
	float dz = a->GetZ() - b->GetZ();

	return dx * dx + dz * dz < dist * dist;
}

// Base class for all spheres (player and enemy)
// Handles movement speed, scaling, scoring and Hyper Mode
class Sphere {
protected:
	IModel* model;
	int points;
	float collisionDistance;
	const float kSphereSpeed = 60.0f;
	float speed;
	float scaleFactor;
	int size;
	float currentScale;
	string defaultSkin;

	bool hyperState;
	float hyperTimer;
	float baseSpeed;
public:
	Sphere(IMesh* mesh, float x, float z, const string& skin) {
		model = mesh->CreateModel(x, 10.0f, z);
		if (!skin.empty()) model->SetSkin(skin.c_str());
		defaultSkin = skin;
		points = 0;
		collisionDistance = 10.0f;
		speed = kSphereSpeed;
		baseSpeed = speed;
		scaleFactor = 1.2f;
		size = 0;
		currentScale = 1.0f;

		hyperState = false;
		hyperTimer = 0.0f;
	}

	IModel* getModel() { return model; }
	float getCollisionDistance() { return collisionDistance; }
	int getPoints() { return points; }

	// Adds points to the sphere
	// Every 40 points the sphere grows in size by a scale factor
	void addPoints() {
		points += 10;
		int newSize = points / 40;
		if (newSize > size) {
			currentScale *= scaleFactor;
			model->ResetScale();
			model->Scale(currentScale);
			collisionDistance *= scaleFactor;
			model->SetY(collisionDistance);
			size = newSize;
		}
	}

	// Activates Hyper Mode for 5 seconds
	// In Hyper Mode the sphere moves faster and attracts nearby cubes
	void activateHyper() {
		if (hyperState)return;
		hyperState = true;
		hyperTimer = 5.0f;
		speed = baseSpeed * 1.2;
		model->SetSkin("hypersphere.jpg");
	}

	// Checks the status of Hyper Mode and turns it off if necessary
	void updateHyper(float dt) {
		if (hyperState) {
			hyperTimer -= dt;
			if (hyperTimer <= 0.0f) {
				hyperState = false;
				speed = baseSpeed;
				model->SetSkin(defaultSkin.c_str());
			}
		}
	}

	bool isHyper() { return hyperState; }
};

// Player-controlled sphere class
// Inherits from Sphere and adds keyboard-based movement and rotation
// Handles user input and updates Hyper Mode state
class PlayerSphere : public Sphere {
	const float kRotationSpeed = 120.0f;
	float rotationSpeed;
public:
	PlayerSphere(IMesh* mesh) : Sphere(mesh, 0, -40, "regularsphere.jpg") {
		rotationSpeed = kRotationSpeed;
	}

	// Processes player input using WASD keys
	// Also updates Hyper Mode state
	void Control(I3DEngine* engine, float dt) {
		updateHyper(dt);
		if (engine->KeyHeld(Key_W)) model->MoveLocalZ(speed * dt);
		if (engine->KeyHeld(Key_S)) model->MoveLocalZ(-speed * dt);
		if (engine->KeyHeld(Key_A)) model->RotateY(-rotationSpeed * dt);
		if (engine->KeyHeld(Key_D)) model->RotateY(rotationSpeed * dt);
	}
};

// Enemy (NPC) sphere class
// Inherits from Sphere and implements automatic movement behaviour
// Can chase targets (cubes or hyper cube) and collect points
class EnemySphere : public Sphere {
	float idleDirection;
public:
	EnemySphere(IMesh* mesh) : Sphere(mesh, 0, 40, "enemysphere.jpg") {
		idleDirection = random(0.0f, 360.0f);
		baseSpeed = baseSpeed / 2;
		speed = baseSpeed;
	}

	// Moves the enemy sphere towards a given target model
	// Used to chase cubes or the Hyper Cube
	void update(IModel* target, float dt) {
		updateHyper(dt);

		if (!target) return;
		model->LookAt(target);
		model->ResetScale();
		model->Scale(currentScale);
		model->MoveLocalZ(speed * dt);
		model->SetY(10.0f * currentScale);
	}

	// Function for enemy movement after the game is over
	void idle(float dt) {
		model->MoveLocalZ(speed * 0.5f * dt);
		model->SetY(10.0f * currentScale);

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

// Manages spawning, respawning and collision logic for all cubes
// Ensures cubes do not spawn too close to each other or spheres
class CubeManager {
	static const int numCubes = 12;
	IModel* cubes[numCubes];
	IMesh* mesh;
	float collisionDistance;
	float distanceBetweenCubes;

	bool tooCloseToCubes(float x, float z) {
		for (int i = 0; i < numCubes; i++) {
			if (!cubes[i])continue;

			float dx = cubes[i]->GetX() - x;
			float dz = cubes[i]->GetZ() - z;

			if (dx * dx + dz * dz < distanceBetweenCubes * distanceBetweenCubes)return true;
		}
		return false;
	}
public:
	CubeManager(IMesh* m) {
		mesh = m;
		collisionDistance = 2.5f;
		distanceBetweenCubes = 10.0f;
		for (int i = 0; i < numCubes; i++)cubes[i] = nullptr;
	}

	void spawnAll(IModel* player, IModel* enemy) {
		for (int i = 0; i < numCubes; i++) {
			float x, z;
			bool valid;
			do {
				x = random(-95.0, 95.0);
				z = random(-95.0, 95.0);

				valid = abs(x - player->GetX()) >= 15 &&
					abs(z - player->GetZ()) >= 15 &&
					abs(x - enemy->GetX()) >= 15 &&
					abs(z - enemy->GetZ()) >= 15 &&
					!tooCloseToCubes(x, z);

			} while (!valid);
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
		bool valid;
		do {
			x = random(-95.0, 95.0);
			z = random(-95.0, 95.0);

			valid = abs(x - player->GetX()) >= 15 &&
				abs(z - player->GetZ()) >= 15 &&
				abs(x - enemy->GetX()) >= 15 &&
				abs(z - enemy->GetZ()) >= 15 &&
				!tooCloseToCubes(x, z);

		} while (!valid);
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

	void getValidSpawnPosition(
		float& x, float& z,
		IModel* player,
		IModel* enemy
	) {
		bool valid;

		do {
			x = random(-95.0f, 95.0f);
			z = random(-95.0f, 95.0f);

			valid =
				abs(x - player->GetX()) >= 15 &&
				abs(z - player->GetZ()) >= 15 &&
				abs(x - enemy->GetX()) >= 15 &&
				abs(z - enemy->GetZ()) >= 15 &&
				!tooCloseToCubes(x, z);

		} while (!valid);
	}
};

// Special cube class used to activate Hyper Mode
// When collected by a sphere, it enables Hyper Mode for a limited time
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
		model->SetPosition(-1000, -1000, -1000);
	}
	float getCollisionDistance() { return collisionDistance; }
};

void main()
{
	// Create a 3D engine (using TLX engine here) and open a window for it
	I3DEngine* myEngine = New3DEngine(kTLX);
	myEngine->StartWindowed();

	// Add default folder for meshes and other media
	//myEngine->AddMediaFolder("C:\\Users\\adm\\TL-Engine\\Media");
	myEngine->AddMediaFolder(".\\Resources");

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
	cubes.getValidSpawnPosition(
		hx, hz,
		player.getModel(),
		enemy.getModel()
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

		// Calculates the time elapsed since the last frame
		// Used for frame-rate independent movement
		deltaTime = myEngine->Timer();

		// Allows the player to quit the game at any time using the Escape key
		if (myEngine->KeyHit(Key_Escape))myEngine->Stop();

		// Switches to the top-down camera view when key '1' is pressed
		if (myEngine->KeyHit(Key_1)) {
			camera->SetPosition(0.0f, 200.0f, 0.0f);
			camera->ResetOrientation();
			camera->RotateLocalX(90.0f);
			isIsometric = false;
		}

		// Switches to the isometric camera view when key '2' is pressed
		if (myEngine->KeyHit(Key_2)) {
			camera->SetPosition(150.0f, 150.0f, -150.0f);
			camera->ResetOrientation();
			camera->RotateLocalY(-45.0f);
			camera->RotateLocalX(45.0f);
			isIsometric = true;
		}

		int screenCenterX = 600;
		int screenCenterY = 300;

		// Handles game behaviour depending on the current game state
		switch (state)
		{
		// Start state
		// Displays the main menu and waits for player input to begin the game
		case Start:
		{	
			titleFont->Draw("SPHERES.IO", screenCenterX - 200, screenCenterY, kWhite);
			menuFont->Draw("by Vladislav Vasilev", screenCenterX + 50, screenCenterY + 80, kLightGrey);
			menuFont->Draw("Press SPACE to Start", screenCenterX - 180, screenCenterY + 150, kRed);

			// Starts the game when SPACE is pressed
			if (myEngine->KeyHit(Key_Space))state = Playing;
			break;
		}
		// Playing state
		// Handles player and enemy movement, collisions,
		// scoring, camera control and win/lose conditions
		case Playing:
		{
			// Processes player input and movement
			player.Control(myEngine, deltaTime);

			// Determines the current target for the enemy sphere
			// Enemy prioritises the Hyper Cube when it is active
			IModel* target = nullptr;
			if (hyper.isActive()) {
				target = hyper.getModel();
			}
			else {
				target = cubes.findClosest(enemy.getModel());
			}

			// Updates enemy movement towards the selected target
			enemy.update(target, deltaTime);

			// Checks collisions between the player and all cubes
			// Awards points and respawns cubes when collected
			for (int i = 0; i < 12; i++) {
				IModel* cube = cubes.getCube(i);
				if (!cube)continue;

				// Player collects cube
				if (Collision(player.getModel(), cube, player.getCollisionDistance() + cubes.getCollisionDistance())) {
					player.addPoints();
					cubes.respawn(i, player.getModel(), enemy.getModel());
				}

				// Enemy collects cube
				if (Collision(enemy.getModel(), cube, enemy.getCollisionDistance() + cubes.getCollisionDistance())) {
					enemy.addPoints();
					cubes.respawn(i, player.getModel(), enemy.getModel());
				}

			}

			// Checks collision with the Hyper Cube
			// Activates Hyper Mode for player or enemy when collected
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

			// Attracts nearby cubes when Hyper Mode is active
			if (!hyper.isActive()) {
				if (player.isHyper()) cubes.attractCubes(player.getModel(), deltaTime);
				if (enemy.isHyper()) cubes.attractCubes(enemy.getModel(), deltaTime);
			}

			// Handles collision between player and enemy spheres
			// Spheres bounce off each other if the point difference is small
			// Otherwise the stronger sphere consumes the weaker one
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

			// Checks win condition for player
			if (player.getPoints() >= 120) state = GameWon;

			// Checks win condition for enemy
			if (enemy.getPoints() >= 120) {
				state = GameOver;
				enemy.getModel()->RotateY(random(90.0f, 360.0f));
			}

			// Allows camera movement in top-down view using arrow keys
			if (!isIsometric) {
				float camMoveSpeed = cameraSpeed * deltaTime;
				if (myEngine->KeyHeld(Key_Up)) camera->MoveZ(camMoveSpeed);
				if (myEngine->KeyHeld(Key_Down)) camera->MoveZ(-camMoveSpeed);
				if (myEngine->KeyHeld(Key_Left)) camera->MoveX(-camMoveSpeed);
				if (myEngine->KeyHeld(Key_Right)) camera->MoveX(camMoveSpeed);
			}

			// Ends the game if the player moves outside the island boundaries
			// This simulates the player falling into the water
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

			// Displays player and enemy scores
			// The higher score is shown first, aligned to the right
			if (player.getPoints() >= enemy.getPoints()) {
				menuFont->Draw(("Player: " + to_string(player.getPoints())).c_str(), 1000, 20, kWhite);
				menuFont->Draw(("Enemy: " + to_string(enemy.getPoints())).c_str(), 1000, 60, kRed);
			}
			else {
				menuFont->Draw(("Enemy: " + to_string(enemy.getPoints())).c_str(), 1000, 60, kRed);
				menuFont->Draw(("Player: " + to_string(player.getPoints())).c_str(), 1000, 20, kWhite);
			}

			// Toggles pause state when 'P' key is pressed
			if (myEngine->KeyHit(Key_P))state = Paused;

			break;
		}
		// Paused state
		// Freezes all movement and displays pause message
		case Paused:
		{
			menuFont->Draw("Game is PAUSED", screenCenterX - 100, screenCenterY, kWhite);
			menuFont->Draw("Press P to UNPAUSE", screenCenterX - 100, screenCenterY + 50, kWhite);
			if (myEngine->KeyHit(Key_P))state = Playing;
			break;
		}
		// GameOver state
		// Displays final scores
		// Enemy sphere continues moving around the island
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
		// GameWon state
		// Displays congratulatory message and final score
		// Player movement is disabled
		case GameWon:
		{
			titleFont->Draw("YOU WON!", screenCenterX - 100, screenCenterY, kGreen);
			menuFont->Draw(("Final Score: " + to_string(player.getPoints())).c_str(), screenCenterX - 100, screenCenterY + 100, kWhite);

			menuFont->Draw("Press ESC to QUIT", screenCenterX - 100, screenCenterY + 150, kWhite);

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
