#ifdef GL_ES
#version 300 es
precision highp float;
#else
#version 330 core
#endif

layout(location = 0) in vec4 pos;

uniform mat4 persp;
uniform mat4 view;
uniform mat4 transform;
uniform float time;
uniform float scale;

out vec2 tc;
out float particleAlpha;

// Better pseudo-random function
float hash(float n) {
	return fract(sin(n) * 43758.5453123);
}

float hash2(vec2 p) {
	return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

void main()
{
	float id = float(gl_InstanceID);

	// Generate better random values per particle
	float rand1 = hash(id);
	float rand2 = hash(id + 123.456);
	float rand3 = hash(id + 789.012);
	float rand4 = hash(id + 345.678);
	float rand5 = hash(id + 901.234);

	// Random spawn time offset for staggered appearance
	float spawnOffset = rand1 * 0.15;
	float localTime = max(0.0, time - spawnOffset);

	vec3 cameraRightWorldSpace = vec3(view[0][0], view[1][0], view[2][0]);
	vec3 cameraUpWorldSpace = vec3(view[0][1], view[1][1], view[2][1]);
	vec4 center = transform * vec4(0.0, 0.0, 0.0, 1.0);

	// Varied particle sizes (grow then shrink more gradually)
	float maxsz = (8.0 + rand2 * 12.0) * scale;
	// Particles grow quickly then shrink slowly
	float sizeProgress = smoothstep(0.0, 0.2, localTime) * (1.0 - smoothstep(0.5, 1.2, localTime));
	float sz = maxsz * max(0.3, sizeProgress); // Minimum size 30% so they don't vanish too quickly

	// Random rotation per particle
	float rotationSpeed = (rand3 - 0.5) * 6.28;
	float rotation = rotationSpeed * localTime;

	vec2 p = vec2(
		pos.x * cos(rotation) - pos.z * sin(rotation),
		pos.x * sin(rotation) + pos.z * cos(rotation)
	);

	vec3 vertPosWorldSpace =
		center.xyz +
		cameraRightWorldSpace * p.x * sz +
		cameraUpWorldSpace * p.y * sz;

	// More random initial direction with added chaos
	float theta = rand4 * 6.28318; // 0 to 2π
	float phi = acos(2.0 * rand5 - 1.0); // Better sphere distribution

	// Add secondary random offset to break symmetry
	float theta2 = hash(id + 567.89) * 6.28318;
	float phi2 = hash(id + 234.56) * 3.14159;

	vec3 randomDir = vec3(
		sin(phi) * cos(theta),
		sin(phi) * sin(theta),
		cos(phi)
	);

	// Add secondary direction to create more chaos
	vec3 randomDir2 = vec3(
		sin(phi2) * cos(theta2),
		sin(phi2) * sin(theta2),
		cos(phi2)
	);

	// Mix directions for less uniform spread
	randomDir = normalize(mix(randomDir, randomDir2, rand3 * 0.7));

	// MUCH more varied initial speeds - includes very slow and very fast particles
	float speedVariation = rand2 * rand2 * 2.0; // 0.0x to 2.0x speed (squared for more variation)
	float initialSpeed = 4.0 + speedVariation * 8.0; // 4 to 20 units/sec (much slower)

	// Some particles get a random "kick" in a different direction
	vec3 kickDir = vec3(
		hash(id + 111.11) - 0.5,
		hash(id + 222.22) - 0.5,
		hash(id + 333.33) - 0.5
	);
	kickDir = normalize(kickDir);
	float kickStrength = hash(id + 444.44) * 3.0; // Reduced from 8.0 to 3.0

	// Initial position offset - much more varied
	vertPosWorldSpace += randomDir * rand3 * 5.0 * scale;
	vertPosWorldSpace += kickDir * hash(id + 555.55) * 3.0 * scale;

	// Physics-based motion with gravity (reduced for slower fall)
	float gravity = -8.0 - rand4 * 4.0; // Varied gravity: -8 to -12 (was -15 to -25)
	float drag = 0.15 + rand5 * 0.6; // Varied drag: 0.15 to 0.75 (less drag = particles travel further)

	// Velocity with drag: v(t) = v0 * e^(-drag*t)
	float dragFactor = exp(-drag * localTime);
	vec3 velocity = randomDir * initialSpeed * dragFactor;

	// Add the random kick velocity
	velocity += kickDir * kickStrength * dragFactor;

	// Add gravity (with some randomness)
	velocity.y += gravity * localTime * (0.8 + rand1 * 0.4);

	// Much stronger turbulence/noise for chaotic motion
	float turbTime1 = localTime * (5.0 + rand1 * 10.0) + rand2 * 100.0;
	float turbTime2 = localTime * (7.0 + rand3 * 8.0) + rand4 * 100.0;
	float turbTime3 = localTime * (6.0 + rand5 * 12.0) + rand1 * 100.0;

	vec3 turbulence = vec3(
		sin(turbTime1) * cos(turbTime2),
		sin(turbTime2) * cos(turbTime3),
		sin(turbTime3) * cos(turbTime1)
	) * 5.0 * (1.0 - localTime * localTime); // Stronger early, fades out

	velocity += turbulence / 10.0; 

	// Apply velocity to position
	vertPosWorldSpace += velocity * localTime * scale;

	gl_Position = persp * view * vec4(vertPosWorldSpace.xyz, 1.0);

	// Pass alpha to fragment shader for varied particle lifetimes (slower fade)
	particleAlpha = 1.0 - smoothstep(0.3, 1.5, localTime); // Start fading later, finish later
	particleAlpha *= (0.6 + rand3 * 0.4); // Vary particle opacity

	tc = pos.xz;
	tc += vec2(1.0, 1.0);
	tc /= 2.0;
}
