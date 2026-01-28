#version 330 core

in vec2 pos2d;
uniform float u_pitch; // Moves horizon up/down
uniform float u_roll;  // Rotates the horizon

out vec4 color;

void main()
{
    float dist = length(pos2d);
    float mask = step(dist, 1.0); // Keep it circular

    // 1. Handle Roll (Rotate the coordinate system)
    float c = cos(u_roll);
    float s = sin(u_roll);
    mat2 rot = mat2(c, -s, s, c);
    vec2 rotatedUV = rot * pos2d;

    // 2. Handle Pitch (Offset the y-position)
    // Adding u_pitch moves the "horizon line" up or down
    float horizon = rotatedUV.y + u_pitch;

    // 3. Define Sky and Ground
    // Use smoothstep for a clean but slightly soft horizon line
    float isSky = smoothstep(-0.01, 0.01, horizon);
    
    vec4 skyCol = vec4(0.0, 0.4, 0.8, 0.7);   // Blue-ish sky
    vec4 groundCol = vec4(0.4, 0.2, 0.0, 0.7); // Brown-ish ground
    vec4 finalColor = mix(groundCol, skyCol, isSky);

    // 4. Add "Pitch Ladder" (Reference lines)
    // Every 0.2 units, draw a horizontal line
    float lines = sin(horizon * 15.0); 
    float lineMask = smoothstep(0.9, 0.95, lines) * step(abs(rotatedUV.x), 0.4);
    finalColor = mix(finalColor, vec4(1.0, 1.0, 1.0, 1.0), lineMask);

    // 5. Static Aircraft Reference (The "W" shape in the center)
    // This part doesn't move with pitch/roll
    float centerWing = step(abs(pos2d.y), 0.02) * step(abs(pos2d.x), 0.3) * step(0.1, abs(pos2d.x));
    float centerDot = step(dist, 0.03);
    
    finalColor = mix(finalColor, vec4(1.0, 1.0, 0.0, 1.0), max(centerWing, centerDot));

    color = finalColor * mask;
}
