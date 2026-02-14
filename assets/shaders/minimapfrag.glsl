#ifdef GL_ES
#version 300 es
precision highp float;
#else
#version 330 core
#endif

in vec2 pos2d;
uniform float u_time;

out vec4 color;

void main()
{
    // Fix inverted Y from the 90-degree X-axis rotation
    vec2 uv = vec2(pos2d.x, -pos2d.y);
    float dist = length(uv);
    float angle = atan(uv.x, uv.y);

    // --- Circular Face ---
    float face = smoothstep(0.96, 0.92, dist);
    vec3 col = vec3(0.04, 0.05, 0.04);

    // --- Outer Bezel ---
    float bezel = smoothstep(0.96, 0.93, dist) - smoothstep(0.90, 0.87, dist);
    col = mix(col, vec3(0.18), bezel);

    // --- Range Rings (static, at 25%, 50%, 75%) ---
    for (int i = 1; i <= 3; i++) {
        float r = float(i) * 0.22;
        float ring = smoothstep(0.008, 0.003, abs(dist - r)) * face;
        col = mix(col, vec3(0.0, 0.35, 0.0), ring * 0.6);
    }

    // --- Crosshair Lines ---
    float crossH = smoothstep(0.006, 0.002, abs(uv.y)) * step(dist, 0.87) * face;
    float crossV = smoothstep(0.006, 0.002, abs(uv.x)) * step(dist, 0.87) * face;
    col = mix(col, vec3(0.0, 0.3, 0.0), (crossH + crossV) * 0.4);

    // --- Compass Tick Marks ---
    float ticks = 0.0;
    // 4 major ticks (N, E, S, W)
    for (int i = 0; i < 4; i++) {
        float ta = float(i) * 1.5708; // PI/2
        float diff = abs(mod(angle - ta + 3.14159, 6.28318) - 3.14159);
        ticks += smoothstep(0.04, 0.02, diff)
               * smoothstep(0.72, 0.74, dist)
               * smoothstep(0.90, 0.87, dist);
    }
    // 8 minor ticks (NE, SE, SW, NW + intermediates)
    for (int i = 0; i < 8; i++) {
        float ta = float(i) * 0.7854; // PI/4
        float diff = abs(mod(angle - ta + 3.14159, 6.28318) - 3.14159);
        ticks += smoothstep(0.02, 0.01, diff)
               * smoothstep(0.80, 0.82, dist)
               * smoothstep(0.90, 0.87, dist);
    }
    col = mix(col, vec3(0.0, 0.7, 0.0), clamp(ticks, 0.0, 1.0) * 0.8);

    // --- Radar Sweep ---
    float sweepSpeed = 2.0;
    float sweepAngle = mod(u_time * sweepSpeed, 6.28318) - 3.14159;
    float angleDiff = mod(angle - sweepAngle + 6.28318, 6.28318);

    // Bright leading edge
    float sweepLine = smoothstep(0.06, 0.01, angleDiff) * step(dist, 0.87) * face;

    // Fading trail behind the sweep
    float trail = smoothstep(1.2, 0.0, angleDiff) * step(dist, 0.87) * face;

    col = mix(col, vec3(0.0, 0.6, 0.0), trail * 0.15);
    col = mix(col, vec3(0.0, 1.0, 0.0), sweepLine * 0.7);

    // --- Outer Edge Glow ---
    float edgeGlow = smoothstep(0.87, 0.85, dist) - smoothstep(0.85, 0.80, dist);
    col = mix(col, vec3(0.0, 0.5, 0.0), edgeGlow * 0.15);

    float alpha = face * 0.75;
    alpha = max(alpha, bezel);

    color = vec4(col, alpha);
}
