#ifdef GL_ES
#version 300 es
precision mediump float;
#else
#version 330 core
#endif

in vec2 pos2d;
uniform float u_time;

out vec4 color;

void main()
{
	float distance = length(pos2d);

	float mask = step(distance, 1.0);

	float rings = sin(distance * 20.0 - u_time * 5.0);

	float greenLine = smoothstep(0.8, 1.0, rings);

	vec4 bgColor = vec4(0.0, 0.0, 0.0, 0.5 * mask);
	vec4 lineColor = vec4(0.0, 0.8, 0.0, 1.0 * mask);

	color = mix(bgColor, lineColor, greenLine);
}
