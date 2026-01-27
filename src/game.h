#pragma once
#include <iostream>
#include "infworld.h"

//Constants
constexpr float SPEED = 48.0f;
constexpr float ACCELERATION = 12.0f;
constexpr float BULLET_SPEED = 384.0f;
constexpr unsigned int MAX_LOD = 5;
constexpr float LOD_SCALE = 2.0f;

constexpr float FOVY = glm::radians(75.0f);
constexpr float ZNEAR = 2.0f;
constexpr float ZFAR = 20000.0f;

constexpr int RANGE = 4;

const glm::vec3 LIGHT = glm::normalize(glm::vec3(-1.0f));

namespace game {
	enum MainMenuActions {
		NONE_SELECTED,
		START_GAME,
		LOAD_GAME,
		OPTIONS,
		CREDITS,
		CHANGE_PLANE_MINUS,
		CHANGE_PLANE_PLUS,
		EXIT_GAME,
	};
	enum PauseMenuActions {
		NONE,
		RESUME,
		EXIT,
		EXIT_TO_MAINMENU,
	};
	enum DeathMenuActions {
		DEATH_NONE,
		TRY_AGAIN,
		DEATH_EXIT,
		DEATH_EXIT_TO_MAINMENU,
	};

	struct Transform {
		glm::vec3 position;
		glm::vec3 scale;
		glm::vec3 rotation; //in radians
		Transform();
		glm::mat4 getTransformMat() const;
		glm::vec3 direction() const; //Forward vector
		glm::vec3 right() const;
		glm::vec3 rotate(const glm::vec3 &v) const;
		glm::vec3 invRotate(const glm::vec3 &v) const;
	};

	struct Timer {
		float time = 0.0f;
		float maxtime = 0.0f;
	};

	struct TimerManager {
		std::unordered_map<std::string, Timer> timers;
		void addTimer(const std::string& name, float maxtime);
		void addTimer(const std::string& name, float time, float maxtime);
		void update(float dt);
		//Resets all timers that have time below 0.0
		void reset();
		bool getTimer(const std::string &name);
	};

	void loadAssets();
	//Initializes the shader uniforms
	void initUniforms();
	void generateChunks(
		const infworld::worldseed &permutations,
		infworld::ChunkTable *chunktables,
		unsigned int range
	);
	void generateNewChunks(
		const infworld::worldseed &permutations,
		infworld::ChunkTable *chunktables,
		infworld::DecorationTable &decorations
	);

	//This is the game loop for "Fight Mode"
	//In fight mode, there are other things in the sky you need to shoot down
	//and some of those things will shoot back at you so you must try to
	//destroy as many things as possible before being shot down yourself
	//returns the final score the player acheives
	game::PauseMenuActions arcadeModeGameLoop();
	//Development Purposes
	game::PauseMenuActions devModeGameLoop();
	//Main menu
	//Returns the game mode selected
	game::MainMenuActions mainMenu();
  // Exit the Game
  void exitGame();

}

namespace gameobjects {
	struct Player {
		game::Transform transform;
		bool crashed;
		enum {
			RY_LEFT,
			RY_RIGHT,
			RY_NONE,
		} yRotationDirection;
		enum {
			RX_UP,
			RX_DOWN,
			RX_NONE,
		} xRotationDirection;
		float deathtimer = 0.0f; //Keeps track of how long the player has been dead
		float shoottimer = 0.0f;
		float speed = 0.0f;
		float fuel = 0.0f;
		//This is used to keep track of the time left for displaying a red screen
		//to indicate to the player that they took damage
		float damagetimer = 0.0f;
		//This keeps track of how long the player has damage immunity
		float damagecooldown = 0.0f;
		unsigned int health;
		Player(glm::vec3 position);

		std::vector<std::string> model_name;
		int current_model;
		void damage(unsigned int amount);
		//Returns the percentage of health left, rounded down
		unsigned int hpPercent();
		//Returns the damagetimer amount left
		float damageTimerProgress();
		void rotateWithMouse(float dt);
		void update(float dt);
		void resetShootTimer();
		void checkIfCrashed(float dt, const infworld::worldseed &permutations);
		void setPlayerObj(int current);
		std::string getPlayerObj() { return model_name[current_model]; }
		int getCurrentIndex() { return current_model; }
	};

	struct Explosion {
		game::Transform transform;
		float timePassed;
		float explosionScale;
		bool visible;
		Explosion(glm::vec3 position);
		Explosion(glm::vec3 position, float scale);
		void update(float dt);
	};

	struct Bullet {
		bool destroyed = false;
		game::Transform transform;
		float time;
		float speed;
		Bullet(const Player &player, const glm::vec3 &offset);
		Bullet(const game::Transform &t, float addspeed, const glm::vec3 &offset);
		Bullet();
		void update(float dt);
	};

	struct Enemy {
		//How many points the player gets if they kill the enemy
		unsigned int scorevalue;
		game::Transform transform;
		std::unordered_map<std::string, float> values;
		int hitpoints;
		Enemy(glm::vec3 position, int hp, unsigned int scoreval);
		void updateBalloon(float dt);
		void updateBlimp(float dt);
		void updateShip(float dt, const Player &player, std::vector<Bullet> &bullets);
		void updateUfo(float dt, const infworld::worldseed &permutations);
		void updatePlane(
			float dt,
			const Player &player,
			std::vector<Bullet> &bullets,
			const infworld::worldseed &permutations
		);
		void checkIfPlaneCrashed(const infworld::worldseed &permutations);
		float getVal(const std::string &key) const;
		void setVal(const std::string &key, float v);
	};

	Enemy spawnBalloon(const glm::vec3 &position, const infworld::worldseed &permutations);
	Enemy spawnBlimp(const glm::vec3 &position, float rotation);
	Enemy spawnShip(const glm::vec3 &position, const infworld::worldseed &permutations);
	Enemy spawnUfo(
		const glm::vec3 &position,
		float rotation,
		const infworld::worldseed &permutations
	);
	Enemy spawnPlane(
		const glm::vec3 &position,
		float rotation,
		const infworld::worldseed &permutations
	);

	struct Props {
			//How many points the player gets if they kill the enemy
			unsigned int scorevalue;
			game::Transform transform;
			std::unordered_map<std::string, float> values;
			int hitpoints;
			Props(glm::vec3 position, int hp, unsigned int scoreval);
			void updateBarrel(float dt);
			float getVal(const std::string &key) const;
			void setVal(const std::string &key, float v);

	};

	Props spawnBarrel(const glm::vec3 &position, const infworld::worldseed &permutations);
}

namespace game {
	//Spawns barrels around the player
	void spawnBarrels(
		gameobjects::Player &player,
		std::vector<gameobjects::Props> &barrels,
		std::minstd_rand0 &lcg,
		const infworld::worldseed &permutations
	);
	//Spawns balloons around the player
	void spawnBalloons(
		gameobjects::Player &player,
		std::vector<gameobjects::Enemy> &balloons,
		std::minstd_rand0 &lcg,
		const infworld::worldseed &permutations
	);
	//Spawns ships around the player
	void spawnShips(
		gameobjects::Player &player,
		std::vector<gameobjects::Enemy> &ships,
		std::minstd_rand0 &lcg,
		const infworld::worldseed &permutations
	);
	//Spawns blimps around the player
	void spawnBlimps(
		gameobjects::Player &player,
		std::vector<gameobjects::Enemy> &blimps,
		std::minstd_rand0 &lcg
	);
	//Spawn ufos around the player
	void spawnUfos(
		gameobjects::Player &player,
		std::vector<gameobjects::Enemy> &ufos,
		std::minstd_rand0 &lcg,
		const infworld::worldseed &permutations
	);
	//Spawn planes around the player
	void spawnPlanes(
		gameobjects::Player &player,
		std::vector<gameobjects::Enemy> &planes,
		std::minstd_rand0 &lcg,
		const infworld::worldseed &permutations,
		float totalTime
	);
	void destroyEnemies(
		gameobjects::Player &player,
		std::vector<gameobjects::Enemy> &enemies,
		std::vector<gameobjects::Explosion> &explosions,
		float explosionscale,
		float crashdist,
		unsigned int &score
	);
	void checkForCollision(
		gameobjects::Player &player,
		std::vector<gameobjects::Enemy> &enemies,
		std::vector<gameobjects::Explosion> &explosions,
		float explosionscale,
		const glm::vec3 &extents
	);
	void destroyProps(
		gameobjects::Player &player,
		std::vector<gameobjects::Props> &props,
		std::vector<gameobjects::Explosion> &explosions,
		float explosionscale,
		float crashdist,
		unsigned int &score
	);
	void checkForCollision(
		gameobjects::Player &player,
		std::vector<gameobjects::Props> &enemies,
		const glm::vec3 &extents
	);
	//Check for collision among enemies
	void checkForCollision(std::vector<gameobjects::Enemy> &enemies, float hitdist);
	//Check for collision among props
	void checkForCollision(std::vector<gameobjects::Props> &props, float hitdist);
	//Returns the position the camera should be following
	glm::vec3 getCameraFollowPos(const Transform &playertransform);
	//Have the camera follow the player	
	void updateCamera(gameobjects::Player &player);
	void updateCamera(gameobjects::Player &player, float dt);
	//Update explosions
	void updateExplosions(
		std::vector<gameobjects::Explosion> &explosions, 
		const glm::vec3 &center,
		float dt
	);
	//Check if the bullet is too far away from the player, if it is, then
	//mark it to be deleted
	//This should preferably be called before `updateBullets`
	void checkBulletDist(
		std::vector<gameobjects::Bullet> &bullets,
		const gameobjects::Player &player
	);
	void updateBullets(std::vector<gameobjects::Bullet> &bullets, float dt);
	void checkForHit(
		std::vector<gameobjects::Bullet> &bullets,
		std::vector<gameobjects::Enemy> &enemies,
		float hitdist
	);
	void checkForHit(
		std::vector<gameobjects::Bullet> &bullets,
		gameobjects::Player &player,
		float hitdist
	);
	void checkForBulletTerrainCollision(
		std::vector<gameobjects::Bullet> &bullets,
		infworld::worldseed &permutations
	);
}

namespace gfx {
	void displaySkybox();
	void displayWater(float totalTime);
	void displayDecorations(infworld::DecorationTable &decorations, float totalTime);
	unsigned int displayTerrain(infworld::ChunkTable *chunktables, int maxlod, float lodscale);	
	void generateDecorationOffsets(infworld::DecorationTable &decorations);
	void displayPlayerPlane(float totalTime, const game::Transform &transform, const std::string& plane_model);
	void displayExplosions(const std::vector<gameobjects::Explosion> &explosions);
	void displayBalloons(const std::vector<gameobjects::Enemy> &balloons);
	void displayBarrels(const std::vector<gameobjects::Props> &barrels);
	void displayShips(const std::vector<gameobjects::Enemy> &ships);
	void displayBlimps(const std::vector<gameobjects::Enemy> &blimps);
	void displayUfos(const std::vector<gameobjects::Enemy> &ufos);
	void displayPlanes(float totalTime, const std::vector<gameobjects::Enemy> &planes);
	void displayBullets(const std::vector<gameobjects::Bullet> &bullets);
	void displayMiniMapBackground();
	void displayEnemyMarkers(
		const std::vector<gameobjects::Enemy> &enemies,
		const game::Transform &playertransform
	);
	void displayCrosshair(const game::Transform &playertransform);
	void displayHUDBackGrounds();
}
