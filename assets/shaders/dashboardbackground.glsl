#ifdef GL_ES
#version 300 es
precision highp float;
#else   
#version 330 core
#endif

out vec4 FragColor;
in vec2 tc; // Assuming UVs from 0.0 to 1.0 for the HUD quad


uniform vec2 resolution;

// Function to create a rounded rectangle shape
float roundedBox(vec2 p, vec2 b, float r) {
    vec2 q = abs(p) - b + r;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r;
}

void main() {
    // 1. Setup coordinates (Shift to center-based for the SDF)
    vec2 uv = tc - 0.5; 
    
    // 2. Define Dashboard Shape (Width 0.4, Height 0.2, Corner Radius 0.02)
    float d = roundedBox(uv, vec2(0.4, 0.15), 0.02);
    
    // 3. Create the Background Fill
    // We use smoothstep for anti-aliasing (clean edges)
    float mask = smoothstep(0.002, 0.0, d);
    
    // 4. Styling the "Aviation Glass"
    vec3 baseColor = vec3(0.02, 0.04, 0.08); // Dark blue-ish tint
    vec3 borderColor = vec3(0.0, 0.8, 1.0);  // Cyan HUD border
    
    // Add a vertical gradient
    baseColor += (tc.y - 0.2) * 0.1;
    
    // 5. Border Logic
    float border = smoothstep(0.005, 0.002, abs(d + 0.005));
    
    // Combine layers
    vec3 finalColor = mix(baseColor, borderColor, border);
    float finalAlpha = mask * 0.85; // Semi-transparent
    
    if (finalAlpha < 0.1) discard; // Don't render transparent pixels
    
    FragColor = vec4(finalColor, finalAlpha);
}
