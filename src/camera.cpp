#define _USE_MATH_DEFINES
#include <math.h>
#include "camera.h"

#include <glm/gtc/matrix_transform.hpp> 

CameraMovement::CameraMovement(
	MovementDirection m,
	MovementDirection s,
	MovementDirection f
) {
	movement = m;
	strafe = s;
	flying = f;
}

Camera::Camera()
{
	position = glm::vec3(0.0f);
	shakeDuration = 0.0f;
	shakeIntensity = 0.0f;
	shakeOffset = glm::vec3(0.0f);
}

Camera::Camera(glm::vec3 pos)
{
	position = pos;
	shakeDuration = 0.0f;
	shakeIntensity = 0.0f;
	shakeOffset = glm::vec3(0.0f);
}

void Camera::initFreeLookFromPlayer(const gameobjects::Player &player)
{
	glm::vec3 offset = position - player.transform.position;
	orbitRadius = glm::length(offset);
	if (orbitRadius < 0.001f)
		orbitRadius = 66.2f;
	orbitPitch = asinf(glm::clamp(offset.y / orbitRadius, -1.0f, 1.0f));
	orbitYaw   = atan2f(offset.x, -offset.z);
	freeLook   = true;
}

void Camera::updateCameraFreeLook(gameobjects::Player &player,
								   float dmousex, float dmousey,
								   float sensitivity)
{
	// Shake passthrough
	if (shakeDuration > 0.0f) {
		float scale = shakeIntensity * (shakeDuration > 0 ? 0.01f : 0.0f);
		shakeOffset = glm::vec3(
			((float)rand()/RAND_MAX * 2.0f - 1.0f) * scale,
			((float)rand()/RAND_MAX * 2.0f - 1.0f) * scale,
			0.0f);
	} else {
		shakeOffset = glm::vec3(0.0f);
	}

	// Rotate orbit with mouse (same sensitivity scale as rotateCamera)
	orbitYaw   += 0.05f * sensitivity * dmousex;
	orbitPitch -= 0.05f * sensitivity * dmousey;   // invert Y: mouse up = camera rises

	constexpr float PITCH_MAX = glm::radians(85.0f);
	orbitPitch = glm::clamp(orbitPitch, -PITCH_MAX, PITCH_MAX);

	// Spherical coordinate → world position offset
	float offsetX =  orbitRadius * cosf(orbitPitch) * sinf(orbitYaw);
	float offsetY =  orbitRadius * sinf(orbitPitch);
	float offsetZ = -orbitRadius * cosf(orbitPitch) * cosf(orbitYaw);

	position = player.transform.position + glm::vec3(offsetX, offsetY, offsetZ);

	// Point camera at player:
	// Camera::forward() = (cos(p)*sin(y), sin(-p), cos(p)*-cos(y))
	// We need forward() == normalize(-offset), so:
	yaw   =  atan2f(offsetX, offsetZ);
	pitch =  asinf(glm::clamp(offsetY / orbitRadius, -1.0f, 1.0f));
}

glm::vec3 Camera::velocity()
{
	glm::vec3 velocity = glm::vec3(0.0f);
	//Move forwards/backwards
	switch(movementDirection)
	{
	case FORWARD:
		velocity += glm::vec3(sinf(yaw), 0.0f, -cosf(yaw));
		break;
	case BACKWARD:
		velocity -= glm::vec3(sinf(yaw), 0.0f, -cosf(yaw));
		break;
	default:
		break;
	}

	//Strafing
	switch(strafeDirection)
	{
	case STRAFE_LEFT:
		velocity += glm::vec3(sinf(-yaw - M_PI / 2.0f), 
							  0.0f,
							  cosf(-yaw - M_PI / 2.0f));
		break;
	case STRAFE_RIGHT:
		velocity += glm::vec3(sinf(-yaw + M_PI / 2.0f), 
							  0.0f,
							  cosf(-yaw + M_PI / 2.0f));
		break;
	default:
		break;
	}

	if(glm::length(velocity) == 0.0f)
		return glm::vec3(0.0f);

	return glm::normalize(velocity);
}

void Camera::fly(float dt, float speed)
{
	//Fly
	switch(flyingDirection)
	{
	case FLY_UP:
		position += glm::vec3(0.0f, speed, 0.0f) * dt;
		break;
	case FLY_DOWN:
		position -= glm::vec3(0.0f, speed, 0.0f) * dt;
		break;
	default:
		break;
	}
}

//Returns the view matrix
glm::mat4 Camera::viewMatrix()
{
	glm::mat4 view =
		   glm::rotate(glm::mat4(1.0f), pitch, glm::vec3(1.0f, 0.0f, 0.0f)) *
		   glm::rotate(glm::mat4(1.0f), yaw, glm::vec3(0.0f, 1.0f, 0.0f)) *
		   glm::translate(glm::mat4(1.0f), -position);

	// Apply shake as angular rotation in view space so the whole screen
	// jitters uniformly instead of translating objects in world space
	if (shakeDuration > 0.0f) {
		view = glm::rotate(glm::mat4(1.0f), shakeOffset.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
			   glm::rotate(glm::mat4(1.0f), shakeOffset.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
			   view;
	}

	return view;
}

glm::vec3 Camera::forward()
{
	return glm::normalize(
		glm::vec3(
			cosf(pitch) * sinf(yaw),
			sinf(-pitch),
			cosf(pitch) * -cosf(yaw)
		)
	);
}

glm::vec3 Camera::right()
{
	return glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), forward()));
}

glm::vec3 Camera::up()
{	
	return glm::normalize(glm::cross(forward(), right()));
}

void Camera::rotateCamera(float dmousex, float dmousey, float sensitivity)
{
	//Rotate the yaw of the camera when the cursor moves left and right
	if(dmousex != 0.0f)
		yaw += 0.05f * sensitivity * dmousex;

	//Rotate the pitch of the camera when the cursor moves up and down
	if(dmousey != 0.0f)
		pitch += 0.05f * sensitivity * dmousey; 

	//Clamp the pitch to be between -pi / 2 radians and pi / 2 radians
	if(pitch < -M_PI / 2.0f)
		pitch = -M_PI / 2.0f;
	if(pitch > M_PI / 2.0f)
		pitch = M_PI / 2.0f;
}

void Camera::shakeCamera(float intensity, float duration) {
	shakeDuration = duration;
	shakeIntensity = intensity;
}

MovementDirection handlePress(MovementDirection currentDir, MovementDirection d)
{
	if(d != NONE)
		return d;
	return currentDir;
}

MovementDirection handleRelease(MovementDirection currentDir, MovementDirection d)
{
	if(d == currentDir)
		return NONE;
	return currentDir;
}

void Camera::updateMovement(CameraMovement m, bool pressed)
{
	if(pressed) {
		movementDirection = handlePress(movementDirection, m.movement);
		strafeDirection = handlePress(strafeDirection, m.strafe);
		flyingDirection = handlePress(flyingDirection, m.flying);
	}
	else {
		movementDirection = handleRelease(movementDirection, m.movement);
		strafeDirection = handleRelease(strafeDirection, m.strafe);
		flyingDirection = handleRelease(flyingDirection, m.flying);
	}
}

glm::vec3 Camera::getCameraFollowPos(const game::Transform &playertransform)
{
	// Only apply yaw and pitch to the camera offset, NOT roll.
	// This prevents the camera from swinging sideways when the plane banks.
	glm::mat4 rotationMat(1.0f);
	rotationMat = glm::rotate(rotationMat, playertransform.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
	rotationMat = glm::rotate(rotationMat, playertransform.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec4 offset = rotationMat * glm::vec4(0.0f, 28.0f, -60.0f, 1.0f);
	return playertransform.position + glm::vec3(offset);
}

void Camera::updateCamera(gameobjects::Player &player)
{
	//Update camera
	position = getCameraFollowPos(player.transform);
	yaw = -(player.transform.rotation.y + glm::radians(180.0f));
	pitch = player.transform.rotation.x;
}

void Camera::updateCamera(gameobjects::Player &player, float dt)
{
	if (shakeDuration > 0) {
		// Generate small random angles (radians) for view-space rotation
		float scale = shakeIntensity * 0.01f * (shakeDuration / (shakeDuration + dt));
		float x = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * scale;
		float y = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * scale;
		shakeOffset = glm::vec3(x, y, 0.0f);
		shakeDuration -= dt;
	} else {
		shakeOffset = glm::vec3(0.0f);
	}
   //Update camera
	position = getCameraFollowPos(player.transform);
	float
		_yaw = -(player.transform.rotation.y + glm::radians(180.0f)),
		_pitch = player.transform.rotation.x;
	yaw += (_yaw - yaw) * 6.0f * dt;
	pitch += (_pitch - pitch) * 7.0f * dt;
}

geo::Frustum Camera::getViewFrustum(float znear, float zfar, float aspect, float fovy)
{
	float halfHeight = zfar * tanf(fovy / 2.0f);
	float halfWidth = halfHeight * aspect;
	return {
		.back = geo::Plane(position + znear * forward(), forward()),
		.front = geo::Plane(position + zfar * forward(), -forward()),
		.top = geo::Plane(position, glm::cross(right(), zfar * forward() + up() * halfHeight)),
		.bottom = geo::Plane(position, glm::cross(zfar * forward() - up() * halfHeight, right())),
		.left = geo::Plane(position, glm::cross(up(), zfar * forward() - right() * halfWidth)),
		.right = geo::Plane(position, glm::cross(zfar * forward() + right() * halfWidth, up())),
	};
}
