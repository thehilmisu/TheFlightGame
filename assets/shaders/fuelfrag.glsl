#ifdef GL_ES
#version 300 es
precision highp float;
#else
#version 330 core
#endif

in vec2 pos2d;
uniform float u_fuelLevel;      // 0 to 100
uniform float u_lowFuelWarning; // e.g., 15.0
uniform float u_time;

out vec4 color;

// Helper to draw a small rectangle line
float drawRect(vec2 uv, vec2 center, vec2 size) {
    vec2 d = abs(uv - center) - size;
    return step(max(d.x, d.y), 0.0);
}
float letterF(vec2 uv, vec2 pos, float s) {
    // We negate the Y offsets (e.g., -0.13 instead of 0.13) to counter the rotation
    float v = drawRect(uv, pos + vec2(0.0, 0.0),   vec2(0.02 * s, 0.15 * s)); // Vertical
    v += drawRect(uv, pos + vec2(0.08 * s, -0.13 * s), vec2(0.08 * s, 0.02 * s)); // Top bar
    v += drawRect(uv, pos + vec2(0.06 * s, -0.03 * s), vec2(0.06 * s, 0.02 * s)); // Mid bar
    return clamp(v, 0.0, 1.0);
}
// Procedural Letter E
float letterE(vec2 uv, vec2 pos, float s) {
    float v = drawRect(uv, pos + vec2(0.0, 0.0),    vec2(0.02 * s, 0.15 * s)); // Vertical
    v += drawRect(uv, pos + vec2(0.08 * s, 0.13 * s),  vec2(0.08 * s, 0.02 * s)); // Top
    v += drawRect(uv, pos + vec2(0.06 * s, 0.0),     vec2(0.06 * s, 0.02 * s)); // Mid
    v += drawRect(uv, pos + vec2(0.08 * s, -0.13 * s), vec2(0.08 * s, 0.02 * s)); // Bot
    return clamp(v, 0.0, 1.0);
}

void main() {
    float fuelPct = u_fuelLevel / 100.0;
    float warnPct = u_lowFuelWarning / 100.0;
    
    // Bar Logic
    float barMask = step(abs(pos2d.x), 0.25) * step(abs(pos2d.y), 0.7);
    float normalizedY = (pos2d.y + 0.7) / 1.4;
    float isFilled = step(1.0 - fuelPct, normalizedY);
    
    // Color & Pulse
    vec3 fuelColor = (fuelPct < warnPct) ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 0.8, 0.0);
    float pulse = (fuelPct < warnPct) ? (0.7 + 0.3 * sin(u_time * 8.0)) : 1.0;
    
    // UI Elements
    float fMark = letterF(pos2d, vec2(-0.45, -0.6), 1.0);
    float eMark = letterE(pos2d, vec2(-0.45, 0.6), 1.0);
    float markings = clamp(fMark + eMark, 0.0, 1.0);
    
    // Final Composition
    vec4 bg = vec4(0.1, 0.1, 0.1, 0.6);
    vec4 fill = vec4(fuelColor * pulse, 1.0);
    vec4 uiColor = vec4(1.0, 1.0, 1.0, 1.0); // White for letters
    
    vec4 barFinal = mix(bg, fill, isFilled) * barMask;
    color = mix(barFinal, uiColor, markings);
}
