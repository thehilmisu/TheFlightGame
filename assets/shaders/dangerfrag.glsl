#ifdef GL_ES
#version 300 es
precision highp float;
#else
#version 330 core
#endif

in vec2 pos2d;
uniform float time;
out vec4 color;

void main() {
    // sin(time) goes from -1 to 1. 
    // abs() or (sin * 0.5 + 0.5) keeps it in the 0 to 1 range.
    float flashfactor = abs(sin(time * 2.0));

    color = vec4(1.0, 0.0, 0.0, 0.2 * flashfactor);
}