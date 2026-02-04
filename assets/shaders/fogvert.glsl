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

out vec3 worldPos;
out vec2 tc;

void main()
{
	// Calculate position in world space
	vec4 p = transform * pos;
	worldPos = p.xyz;

	// Standard projection
	gl_Position = persp * view * p;

	// Optional: Texture coords for noise/clouds
	tc = pos.xz * 0.5 + 0.5;
}
