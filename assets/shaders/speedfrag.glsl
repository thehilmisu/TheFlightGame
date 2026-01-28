#version 330 core

in vec2 pos2d;
uniform float u_speed;    // Current speed
uniform float u_maxSpeed; // e.g., 500.0

out vec4 color;

// Helper to draw a rectangle
float drawRect(vec2 uv, vec2 center, vec2 size) {
    vec2 d = abs(uv - center) - size;
    return step(max(d.x, d.y), 0.0);
}

void main() {
    // 1. Normalize speed (0.0 to 1.0)
    float speedPct = clamp(u_speed / u_maxSpeed, 0.0, 1.0);

    // 2. Vertical Track Geometry (The background bar)
    // Centered at x=0, height from -0.8 to 0.8
    float trackMask = drawRect(pos2d, vec2(0.0, 0.0), vec2(0.1, 0.8));

    // 3. Handle Logic (The moving horizontal bar)
    // Map speedPct (0 to 1) to the Y coordinate range (-0.8 to 0.8)
    float handleY = -0.8 + (speedPct * 1.6);
    
    // Draw a horizontal handle that is wider than the track
    float handle = drawRect(pos2d, vec2(0.0, handleY), vec2(0.25, 0.02));

    // 4. Tick Marks (Static scale)
    // Draw small lines every 10% of the gauge
    float ticks = 0.0;
    for(float i = -0.8; i <= 0.81; i += 0.16) {
        ticks += drawRect(pos2d, vec2(0.15, i), vec2(0.05, 0.005));
    }

    // 5. Coloring
    vec4 trackCol = vec4(0.1, 0.1, 0.1, 0.6);   // Dark track
    vec4 handleCol = vec4(1.0, 1.0, 0.0, 1.0);  // Bright yellow handle
    vec4 tickCol = vec4(1.0, 1.0, 1.0, 0.8);    // White ticks

    // Layering: Track -> Ticks -> Handle
    vec4 finalColor = mix(vec4(0.0), trackCol, trackMask);
    finalColor = mix(finalColor, tickCol, ticks);
    finalColor = mix(finalColor, handleCol, handle);

    color = finalColor;
}
