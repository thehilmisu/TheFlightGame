#version 330 core

in vec2 pos2d;
uniform float u_value; // Current Altitude or Speed

out vec4 color;

void main()
{
    // 1. Setup the Tape Geometry
    // Usually tapes are tall rectangles. We can mask the edges.
    float tapeMask = step(abs(pos2d.x), 0.4); // Thin vertical strip

    // 2. Scrolling Logic
    // We scale the value to control how "dense" the markings are.
    // 0.01 means one major tick every 100 units (feet/knots).
    float scrollPos = pos2d.y + (u_value * 0.01); 
    
    // 3. Draw the Tick Marks (Major and Minor)
    // Major ticks every 1.0 unit in scroll space
    float majorTicks = sin(scrollPos * 6.2831); // 2*PI for perfect looping
    float tickLine = smoothstep(0.9, 0.98, majorTicks);
    
    // Adjust tick width: make lines wider for major increments
    float tickWidth = step(abs(pos2d.x), 0.35); 
    float finalTicks = tickLine * tickWidth;

    // 4. Colors
    vec4 bgColor = vec4(0.1, 0.1, 0.1, 0.8 * tapeMask); // Dark semi-transparent grey
    vec4 tickColor = vec4(1.0, 1.0, 1.0, 1.0 * tapeMask); // White lines
    
    // 5. Fixed Central Pointer
    // This triangle/line stays still while the tape moves behind it
    float pointer = step(abs(pos2d.y), 0.02) * step(pos2d.x, -0.3);

    vec3 finalRGB = mix(bgColor.rgb, tickColor.rgb, finalTicks);
    finalRGB = mix(finalRGB, vec4(1.0, 1.0, 0.0, 1.0).rgb, pointer); // Yellow pointer

    color = vec4(finalRGB, bgColor.a + finalTicks + pointer);
}
