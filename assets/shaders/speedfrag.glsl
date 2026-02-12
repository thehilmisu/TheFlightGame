#ifdef GL_ES
#version 300 es
precision highp float;
#else
#version 330 core
#endif

in vec2 pos2d;
uniform float u_speed;    // Current speed
uniform float u_maxSpeed; // e.g., 150.0

out vec4 color;

void main() {
    // Fix inverted Y caused by the 90-degree X-axis rotation in the transform
    vec2 uv = vec2(pos2d.x, -pos2d.y);
    float speedPct = clamp(u_speed / u_maxSpeed, 0.0, 1.0);

    float dist = length(uv);
    // Angle: 0 at top, negative = left, positive = right
    float angle = atan(uv.x, uv.y);

    // Arc sweep: ~264 degrees total
    float arcStart = -2.3;
    float arcEnd = 2.3;
    float arcSpan = arcEnd - arcStart;

    // --- Gauge Face ---
    float face = smoothstep(0.96, 0.92, dist);
    vec3 col = vec3(0.06);

    // --- Outer Bezel ---
    float bezel = smoothstep(0.96, 0.93, dist) - smoothstep(0.90, 0.87, dist);
    col = mix(col, vec3(0.2), bezel);

    // --- Major Tick Marks (6 ticks: 0, 30, 60, 90, 120, 150) ---
    float ticks = 0.0;
    for (int i = 0; i <= 5; i++) {
        float t = float(i) / 5.0;
        float ta = arcStart + t * arcSpan;
        ticks += smoothstep(0.03, 0.012, abs(angle - ta))
               * smoothstep(0.58, 0.62, dist)
               * smoothstep(0.87, 0.85, dist);
    }

    // --- Minor Tick Marks (every 10 units = 15 intervals) ---
    for (int i = 0; i <= 15; i++) {
        float t = float(i) / 15.0;
        float ta = arcStart + t * arcSpan;
        ticks += smoothstep(0.014, 0.006, abs(angle - ta))
               * smoothstep(0.70, 0.73, dist)
               * smoothstep(0.87, 0.85, dist);
    }
    col = mix(col, vec3(0.85), clamp(ticks, 0.0, 1.0));

    // --- Colored Fill Arc ---
    float needleAngle = arcStart + speedPct * arcSpan;
    float inArc = step(arcStart, angle) * step(angle, arcEnd);
    float arcBand = smoothstep(0.86, 0.84, dist) - smoothstep(0.79, 0.77, dist);
    float fillMask = step(angle, needleAngle) * inArc * arcBand;

    // Blue -> cyan -> yellow -> red gradient
    float normPos = clamp((angle - arcStart) / arcSpan, 0.0, 1.0);
    vec3 arcColor;
    if (normPos < 0.6) {
        arcColor = mix(vec3(0.1, 0.5, 0.9), vec3(0.1, 0.85, 0.8), normPos / 0.6);
    } else if (normPos < 0.8) {
        arcColor = mix(vec3(0.1, 0.85, 0.8), vec3(1.0, 0.7, 0.0), (normPos - 0.6) / 0.2);
    } else {
        arcColor = mix(vec3(1.0, 0.7, 0.0), vec3(0.9, 0.1, 0.0), (normPos - 0.8) / 0.2);
    }
    col = mix(col, arcColor, fillMask);

    // Dim outline for unfilled portion
    float emptyMask = step(needleAngle, angle) * inArc * arcBand;
    col = mix(col, vec3(0.14), emptyMask);

    // --- Danger Zone Marks (top 20% of arc) ---
    float dangerStart = arcStart + 0.8 * arcSpan;
    float dangerBand = smoothstep(0.87, 0.86, dist) - smoothstep(0.85, 0.84, dist);
    float dangerMask = step(dangerStart, angle) * step(angle, arcEnd) * dangerBand;
    col = mix(col, vec3(0.8, 0.1, 0.0), dangerMask);

    // --- Needle ---
    float needleMask = smoothstep(0.022, 0.007, abs(angle - needleAngle))
                     * smoothstep(0.09, 0.12, dist)
                     * smoothstep(0.77, 0.74, dist);
    col = mix(col, vec3(1.0, 0.15, 0.0), needleMask);

    // --- Center Pivot ---
    float pivot = smoothstep(0.11, 0.09, dist);
    col = mix(col, vec3(0.22), pivot);
    float pivotInner = smoothstep(0.06, 0.04, dist);
    col = mix(col, vec3(0.35), pivotInner);

    color = vec4(col, face);
}
