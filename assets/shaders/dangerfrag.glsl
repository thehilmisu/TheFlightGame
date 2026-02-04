#ifdef GL_ES
#version 300 es
precision highp float;
#else
#version 330 core
#endif

in vec2 pos2d;
out vec4 color;

void main() {
    color = (1.0, 0.0, 0.0, 0.5);
}
