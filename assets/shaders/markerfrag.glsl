#ifdef GL_ES
#version 300 es
precision highp float;
#else
#version 330 core
#endif

in vec2 pos2d;
uniform vec3 u_color;

out vec4 color;

void main() {
    float dist = length(pos2d);

    // Filled circle with smooth edge
    float circle = smoothstep(0.9, 0.6, dist);

    // Soft glow around it
    float glow = smoothstep(1.0, 0.3, dist) * 0.3;

    float alpha = max(circle, glow);
    vec3 col = u_color;

    color = vec4(col, alpha);
}
