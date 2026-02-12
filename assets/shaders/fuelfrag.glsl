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

     // 1. Setup Coordinates
     float dist = length(pos2d);
     float angle = atan(pos2d.x, -pos2d.y); // Bottom is 0

     // 2. Define the Gauge Face (Black background)
     float faceMask = smoothstep(0.92, 0.9, dist); // Main circular face
     vec3 baseColor = vec3(0.05, 0.05, 0.05); // Deep black

     // 3. Draw Tick Marks (Top Half Arc)
     // We use a modulo on the angle to create repeating marks
     float tickAngle = atan(pos2d.x, pos2d.y); // Top is 0
     float ticks = 0.0;
     if (dist > 0.7 && dist < 0.85 && abs(tickAngle) < 2.0) {
         // Create 5 main ticks by checking angle intervals
         float slice = (tickAngle + 2.0) / 4.0; // Map -2.0 to 2.0 -> 0 to 1
         float pattern = fract(slice * 4.0 + 0.5); // 4 intervals = 5 ticks
         ticks = smoothstep(0.1, 0.05, abs(pattern - 0.5));
     }

     // 4. The Red Needle
     // Map fuel 0.0-1.0 to angle -2.0 to 2.0 (Empty to Full)
     float needleTargetAngle = -2.0 + (fuelPct * 4.0);
     float needleMask = smoothstep(0.02, 0.01, abs(tickAngle - needleTargetAngle))
                      * step(dist, 0.85);

     // 5. Center Cap (The black circle holding the needle)
     float centerCap = smoothstep(0.16, 0.14, dist);

     // 6. Labels (F, E, 1/2) - Using your existing procedural letter functions
     float eMark = letterE(pos2d, vec2(-0.4, 0.2), 0.8);
     float fMark = letterF(pos2d, vec2(0.4, 0.2), 0.8);
     // Note: Add a "1/2" logic or texture here for the top center

     // Final Composition
     vec3 finalRGB = baseColor;
     finalRGB = mix(finalRGB, vec3(1.0), ticks);      // White ticks
     finalRGB = mix(finalRGB, vec3(1.0), eMark + fMark); // White letters
     finalRGB = mix(finalRGB, vec4(1.0, 0.0, 0.0, 1.0).rgb, needleMask); // Red needle
     finalRGB = mix(finalRGB, vec3(0.1), centerCap);  // Dark center cap

     color = vec4(finalRGB * faceMask, faceMask);

}
// void main() {
//      float fuelPct = u_fuelLevel / 100.0;
//      float warnPct = u_lowFuelWarning / 100.0;

//      // 1. Convert Cartesian (x,y) to Polar (angle, distance)
//      float angle = atan(pos2d.x, -pos2d.y); // Range -PI to PI
//      float dist = length(pos2d);

//      // 2. Define Gauge Shape (an arc between 0.3 and 0.5 distance)
//      // We limit the angle to a semi-circle (approx -2.2 to 2.2 radians)
//      float gaugeMask = step(0.35, dist) * step(dist, 0.5) * step(abs(angle), 2.2);

//      // 3. Map angle to 0.0 - 1.0 range for the fuel fill
//      // -2.2 is Empty, 2.2 is Full
//      float normalizedAngle = (angle + 2.2) / 4.4;
//      float isFilled = step(normalizedAngle, fuelPct);

//      // 4. Colors & Low Fuel Pulse
//      vec3 fuelColor = (fuelPct < warnPct) ? vec3(1.0, 0.1, 0.1) : vec3(1.0, 0.8, 0.2); // Retro Orange/Yellow
//      float pulse = (fuelPct < warnPct) ? (0.6 + 0.4 * sin(u_time * 10.0)) : 1.0;

//      // 5. Lettering (F and E at the ends of the arc)
//      float eMark = letterE(pos2d, vec2(-0.4, 0.4), 0.7); // Left side
//      float fMark = letterF(pos2d, vec2(0.35, 0.4), 0.7); // Right side
//      float markings = clamp(fMark + eMark, 0.0, 1.0);

//      // 6. Needle Logic (A thin line pointing at the current level)
//      float needleAngle = -2.2 + (fuelPct * 4.4);
//      float needleDiff = abs(angle - needleAngle);
//      float needle = step(needleDiff, 0.05) * step(dist, 0.45);

//      // Final Composition
//      vec4 bg = vec4(0.05, 0.05, 0.05, 0.7); // Darker glass look
//      vec4 fill = vec4(fuelColor * pulse, 1.0);
//      vec4 white = vec4(1.0, 1.0, 1.0, 1.0);

//      // Mix background, the fuel arc, the needle, and the letters
//      vec4 finalGauge = mix(bg, fill, isFilled) * gaugeMask;
//      finalGauge = mix(finalGauge, white, needle); // Add white needle

//      color = mix(finalGauge, white, markings);
// }
// void main() {
//     float fuelPct = u_fuelLevel / 100.0;
//     float warnPct = u_lowFuelWarning / 100.0;
    
//     // Bar Logic
//     float barMask = step(abs(pos2d.x), 0.25) * step(abs(pos2d.y), 0.7);
//     float normalizedY = (pos2d.y + 0.7) / 1.4;
//     float isFilled = step(1.0 - fuelPct, normalizedY);
    
//     // Color & Pulse
//     vec3 fuelColor = (fuelPct < warnPct) ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 0.8, 0.0);
//     float pulse = (fuelPct < warnPct) ? (0.7 + 0.3 * sin(u_time * 8.0)) : 1.0;
    
//     // UI Elements
//     float fMark = letterF(pos2d, vec2(-0.45, -0.6), 1.0);
//     float eMark = letterE(pos2d, vec2(-0.45, 0.6), 1.0);
//     float markings = clamp(fMark + eMark, 0.0, 1.0);
    
//     // Final Composition
//     vec4 bg = vec4(0.1, 0.1, 0.1, 0.6);
//     vec4 fill = vec4(fuelColor * pulse, 1.0);
//     vec4 uiColor = vec4(1.0, 1.0, 1.0, 1.0); // White for letters
    
//     vec4 barFinal = mix(bg, fill, isFilled) * barMask;
//     color = mix(barFinal, uiColor, markings);
// }
