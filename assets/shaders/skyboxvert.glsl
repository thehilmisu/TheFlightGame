#ifdef GL_ES
#version 300 es
precision highp float;
#else
#version 330 core
#endif

layout(location = 0) in vec4 pos;

uniform mat4 persp;
uniform mat4 view;

out vec3 fragpos;

void main()
{
	vec4 p = persp * view * pos;
	gl_Position = p.xyww;
	fragpos = pos.xyz;
}
