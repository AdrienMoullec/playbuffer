#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"
#include <iostream>
#include <string>
#include <ctime>

//Alien Movement based on time
clock_t timeNow;
clock_t timePassed;
clock_t timeCheckpoint;

//Bullet Movement based on time
clock_t timeNowBullet;
clock_t timePassedBullet;
clock_t timeCheckpointBullet;

clock_t endGameTime;
clock_t endGameTimePassed;
bool stop = false;

int DISPLAY_WIDTH = 1000;
int DISPLAY_HEIGHT = 750;
int DISPLAY_SCALE = 1;

void CannonOptions();
void AlienFlight();
void CollisionCheck();
void LevelWinCheck(); //Checks if alien has reached the ground or all the aliens have been defeated
void EndGame();

std::string direction;
int alienVelocity = 15; //15
int alienDescentVelocity = 15;
int alienStepTime = 1000;
bool alienTurnAround;

int gameScore=0;
int levelNumber = 1;
std::string gameState = "Game";

//ID Types allow Object location and edits.
enum GameObjectIDs
{
	PLAYER_BULLETS,
	FLYING_ALIEN,
	CANNON,
	GROUND,
};

// The entry point for a PlayBuffer program
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::CreateGameObject(CANNON, { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 50 }, 20, "player");
	timeCheckpoint = clock();
	timeCheckpointBullet = clock();
	//Setting initial characteristics of Aliens.
	direction = "Right";
	for (int i = 0 ; i <= 6 ; i = i + 1) {
		for (int j = 0; j <= 4; j = j + 1) {
			int id = Play::CreateGameObject(FLYING_ALIEN, { i * 100 , (j * 75) }, 20, "alien1");
		}
	}
	Play::CreateGameObject(GROUND, { DISPLAY_WIDTH / 2,DISPLAY_HEIGHT+110 },0,"ground");
	Play::CentreAllSpriteOrigins();
}

// Repeatedly calls functions to check for object updates and key presses
bool MainGameUpdate(float elapsedTime)
{
	Play::ClearDrawingBuffer(Play::cGrey);

	//moves cannon left and right or stop
	if (gameState == "Game") {
			//Writes out game stuff :)
			Play::DrawFontText("64px", "LEVEL: " + std::to_string(levelNumber), { 50, 30 }, Play::CENTRE);
			Play::DrawFontText("64px", "GAMESCORE: " + std::to_string(gameScore), { DISPLAY_WIDTH - 150, 30 }, Play::CENTRE);
			CannonOptions();
			AlienFlight();
			CollisionCheck();
			LevelWinCheck();
		
	}
	else if (gameState == "Dead") {
		EndGame();
		Play::DrawFontText("64px", "GAMEOVER", { DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 }, Play::CENTRE);
	}
	GameObject& Ground = Play::GetGameObjectByType(GROUND);
	Play::DrawObject(Ground);

	Play::PresentDrawingBuffer();
	return (Play::KeyDown(VK_ESCAPE));
}

// Gets called once when the player quits the game
int MainGameExit(void)
{
	Play::DestroyManager();
	return PLAY_OK;
}

//Movement of cannon along ground & spawning of bullets
void CannonOptions()
{
	GameObject& cannon = Play::GetGameObjectByType(CANNON);
	Vector2D movement = cannon.velocity;  //set previously calculated velocity.
	Vector2D playerPosition = cannon.pos;


	//Options for Player Input
	if (Play::KeyDown(VK_RIGHT))
	{
		movement.x = 5;
	}
	else if (Play::KeyDown(VK_LEFT))
	{
		movement.x = -5;
	}
	else {
		movement.x = 0;
	}
	if (playerPosition.x < 40) {
		movement.x = 0;
		playerPosition.x = 40;
	}
	else if (playerPosition.x > 960) {
		movement.x = 0;
		playerPosition.x = 960;
	}
	if (Play::KeyPressed(VK_SPACE)) {
		int currentShell = Play::CreateGameObject(PLAYER_BULLETS, {playerPosition.x - 5,playerPosition.y-30 }, 10, "bullet");
		//Play::GetGameObject(currentShell).velocity = { 0,-10 };
	}

	//Set Cannon variables
	cannon.velocity = movement;
	cannon.pos = playerPosition;

	Play::UpdateGameObject(cannon);
	Play::DrawObject(cannon);
	//Play::UpdateGameObject(alienInitial);
	//Play::DrawObject(alienInitial);
	std::vector<int> allBullets = Play::CollectGameObjectIDsByType(PLAYER_BULLETS);
	timeNowBullet = clock();
	timePassedBullet = timeNowBullet - timeCheckpointBullet;

	//Sets position of bullets
	if (timePassedBullet > 100) {
		timeCheckpointBullet = clock();
		for (int bulletInt : allBullets) {
			GameObject& bullet = Play::GetGameObject(bulletInt);
			bullet.pos.y = bullet.pos.y - 15;
		}
	}
	//Draws Bullets and destroys them when they disappear
	for (int bulletInt : allBullets) {
		GameObject& bullet = Play::GetGameObject(bulletInt);
		Play::UpdateGameObject(bullet);
		Play::DrawObject(bullet);
		if (!Play::IsVisible(bullet)) {
			Play::DestroyGameObject(bulletInt);
		}
	}
	
}

void AlienFlight() {
	
	//Checks if aliens are near edge of screen
	std::vector<int> allAliens = Play::CollectGameObjectIDsByType(FLYING_ALIEN);
	timeNow = clock();
	timePassed = timeNow - timeCheckpoint;
	for (int alienInt : allAliens) {
		GameObject& alien = Play::GetGameObject(alienInt);
		if ((alien.pos.x > DISPLAY_WIDTH - 30 && direction == "Right") || (alien.pos.x < 30 && direction == "Left")) {
			alienTurnAround = true;
		}
	}

	//Alien horizontal movement
	if (timePassed > alienStepTime / 5 && !alienTurnAround) {
		timeCheckpoint = clock();
		for (int alienInt : allAliens) {
			GameObject& alien = Play::GetGameObject(alienInt);
			if (!alienTurnAround) {
				if (direction == "Right") {
					alien.pos = { alien.pos.x + alienVelocity, alien.pos.y };
				}
				else if (direction == "Left") {
					alien.pos = { alien.pos.x - alienVelocity, alien.pos.y };

				}
			}
		}

	}

	for (int alienInt : allAliens) {
		GameObject& alien = Play::GetGameObject(alienInt);
		if (alienTurnAround) {
			alien.pos = { alien.pos.x,alien.pos.y + alienDescentVelocity };
		}
		if (alien.pos.y > DISPLAY_HEIGHT - 50) {
			gameState = "Dead";
			endGameTime = clock();
		}
		Play::UpdateGameObject(alien);
		Play::DrawObject(alien);
	}
	//Turns aliens around
	if (alienTurnAround) {
		if (direction == "Right") {
			direction = "Left";
		}
		else if (direction == "Left") {
			direction = "Right";
		}
		alienTurnAround = false;
	}

	
}

void CollisionCheck() {

	//Checks collisions between each alien and bullet
	std::vector<int> allAliens = Play::CollectGameObjectIDsByType(FLYING_ALIEN);
	std::vector<int> allBullets = Play::CollectGameObjectIDsByType(PLAYER_BULLETS);
	int collisionIDCheckAlien = -1;
	bool areColliding = false;
	int tempAlienID;
	int tempBulletID;
	for (int alienInt : allAliens) {
		GameObject& Alien = Play::GetGameObject(alienInt);
		collisionIDCheckAlien++;
		int collisionIDCheckBullet = -1;
		for (int bulletInt : allBullets) {
			GameObject& Bullet = Play::GetGameObject(bulletInt);
			collisionIDCheckBullet++;
			if (Play::IsColliding(Alien, Bullet)) {
				tempAlienID = alienInt;
				tempBulletID = bulletInt;
				areColliding = true;
				gameScore += 200;
			}
		}
	}
	if (areColliding) {
		Play::DestroyGameObject(tempAlienID);
		Play::DestroyGameObject(tempBulletID);
	}
}

//Checks to see if all aliens have been defeated
void LevelWinCheck() {

	int alienChecker=0;
	std::vector<int> allAliens = Play::CollectGameObjectIDsByType(FLYING_ALIEN);
	for (int alienInt : allAliens) {
		alienChecker++;
	}
	if (alienChecker == 0) {
		Play::DestroyGameObjectsByType(PLAYER_BULLETS);
		GameObject& cannon = Play::GetGameObjectByType(CANNON);
		cannon.pos = { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 50 };
		timeCheckpoint = clock();
		timeCheckpointBullet = clock();
		//Setting initial characteristics of Aliens.
		direction = "Right";
		for (int i = 30; i <= 630; i = i + 100) {
			for (int j = 30; j <= 410; j = j + 100) {
				int id = Play::CreateGameObject(FLYING_ALIEN, { i,j }, 10, "alien1");
			}
		}
		levelNumber++;
		Play::CentreAllSpriteOrigins();
	}

}


void EndGame() {
	std::vector<int> allAliens = Play::CollectGameObjectIDsByType(FLYING_ALIEN);
	
}