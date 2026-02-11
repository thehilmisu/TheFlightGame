#ifdef GL_ES
#version 300 es
precision highp float;
#else
#version 330 core
#endif

// Inputs from your vertex shader
in vec2 tc;
in vec3 fragpos;
in vec2 pos2d;

// Standard uniform for animation
uniform float time; // Ensure you pass your time variable here

out vec4 fragColor;

float hash(float x) {
    return fract(sin(x) * 43758.5453123);
}

void main()
{
    // 1. Use the texture coordinates 'tc' from your vertex shader
    vec2 uv = tc;
    
    // 2. Your normalized grey color (194, 213, 220)
    vec3 skyColor = vec3(0.761, 0.835, 0.863);
    
    // 3. Rain Logic
    float columns = 1000.0; // More columns = thinner rain
    float id = floor(uv.x * columns);
    float h = hash(id);
    
    // Animate downward using your 'time' uniform
    float speed = h * 0.8 + 0.5;
    float rainY = uv.y - time * speed;
    
    // Create streak pattern
    float streak = fract(rainY * 4.0 + h);
    float drop = smoothstep(0.7, 1.0, streak);
    
    // 4. Final Color Assembly
    // We multiply 'drop' by a lane-alpha to make some lanes fainter than others
    float opacity = h * 0.3; 
    vec3 finalColor = mix(skyColor, vec3(1.0), drop * opacity);
    
    fragColor = vec4(finalColor, drop * opacity);
}
