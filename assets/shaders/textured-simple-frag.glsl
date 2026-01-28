#ifdef GL_ES
#version 300 es
precision mediump float;
#else
#version 330 core
#endif

uniform sampler2D tex;
in vec2 tc;
out vec4 color;

void main()
{
	color = texture(tex, tc);
}
