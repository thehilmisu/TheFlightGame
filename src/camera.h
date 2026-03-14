#ifndef __CAMERA_H__
#include <glm/glm.hpp>
#include "geometry.h"
#include "game.h"

enum MovementDirection {
	NONE,
	FORWARD,
	BACKWARD,
	STRAFE_LEFT,
	STRAFE_RIGHT,
	FLY_UP,
	FLY_DOWN
};

struct CameraMovement {
	MovementDirection 
		movement = NONE,
		strafe = NONE,
		flying = NONE; 
	CameraMovement(MovementDirection m, MovementDirection s, MovementDirection f);
};

MovementDirection handlePress(MovementDirection currentDir, MovementDirection d);
MovementDirection handleRelease(MovementDirection currentDir, MovementDirection d);

struct Camera {
	glm::vec3 position = glm::vec3(0.0f);
	float pitch = 0.0f, yaw = 0.0f;
	float shakeIntensity;
	float shakeDuration;
	glm::vec3 shakeOffset;
	bool  freeLook    = false;
	float orbitYaw    = 0.0f;
	float orbitPitch  = 0.0f;
	float orbitRadius = 66.2f;

	MovementDirection 
		movementDirection = NONE,
		strafeDirection = NONE,
		flyingDirection = NONE;

	glm::mat4 viewMatrix();
	glm::vec3 forward();
	glm::vec3 right();
	glm::vec3 up();

	Camera();
	Camera(glm::vec3 pos);
	glm::vec3 velocity();
	void fly(float dt, float speed);
	void shakeCamera(float intensity, float duration);
	void rotateCamera(float dmousex, float dmousey, float sensitivity);
	void updateMovement(CameraMovement m, bool pressed);
	//Returns the position the camera should be following
	glm::vec3 getCameraFollowPos(const game::Transform &playertransform);
	//Have the camera follow the player	
	void updateCamera(gameobjects::Player &player);
	void updateCamera(gameobjects::Player &player, float dt);
	void initFreeLookFromPlayer(const gameobjects::Player &player);
	void updateCameraFreeLook(gameobjects::Player &player, float dmousex, float dmousey, float sensitivity);
	geo::Frustum getViewFrustum(float znear, float zfar, float aspect, float fovy);
};

#define __CAMERA_H__
#endif
