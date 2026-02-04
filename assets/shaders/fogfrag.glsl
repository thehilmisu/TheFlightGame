#ifdef GL_ES
#version 300 es
precision highp float;
#else
#version 330 core
#endif

in vec3 worldPos;
in vec2 tc;

uniform vec3 camPos;
uniform float fogStart;   // Distance where fog begins (e.g., 500.0)
uniform float fogEnd;     // Distance where fog is max (e.g., 10000.0)

out vec4 color;

void main()
{
	// 1. Calculate distance from camera to the fragment
	float dist = distance(worldPos, camPos);

	// 2. Calculate fog density based on distance
	// Returns 0.0 at fogStart and 1.0 at fogEnd
	float fogAmount = smoothstep(fogStart, fogEnd, dist);

	// 3. Optional: Make the fog feel "heavier" (non-linear)
	fogAmount = pow(fogAmount, 0.7);

	// 4. Fog color - light atmospheric haze
	vec3 fogColor = vec3(0.8, 0.85, 0.9);

	// Final color with alpha transparency
	// color = vec4(fogColor, fogAmount * 0.9);
	color = vec4(1.0, 0.0, 0.0, 1.0);
}
