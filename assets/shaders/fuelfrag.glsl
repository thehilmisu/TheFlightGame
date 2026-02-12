#ifdef GL_ES
#version 300 es
precision highp float;
#else
#version 330 core
#endif

in vec2 pos2d;
uniform float u_fuelLevel;      // 0 to 100
uniform float u_lowFuelWarning; // e.g., 0.15
uniform float u_time;

out vec4 color;

float drawRect(vec2 uv, vec2 center, vec2 size) {
    vec2 d = abs(uv - center) - size;
    return step(max(d.x, d.y), 0.0);
}

float letterF(vec2 uv, vec2 pos, float s) {
    float v = drawRect(uv, pos, vec2(0.02 * s, 0.15 * s));
    v += drawRect(uv, pos + vec2(0.08 * s, 0.13 * s), vec2(0.08 * s, 0.02 * s));
    v += drawRect(uv, pos + vec2(0.06 * s, 0.03 * s), vec2(0.06 * s, 0.02 * s));
    return clamp(v, 0.0, 1.0);
}

float letterE(vec2 uv, vec2 pos, float s) {
    float v = drawRect(uv, pos, vec2(0.02 * s, 0.15 * s));
    v += drawRect(uv, pos + vec2(0.08 * s, 0.13 * s), vec2(0.08 * s, 0.02 * s));
    v += drawRect(uv, pos + vec2(0.06 * s, 0.0), vec2(0.06 * s, 0.02 * s));
    v += drawRect(uv, pos + vec2(0.08 * s, -0.13 * s), vec2(0.08 * s, 0.02 * s));
    return clamp(v, 0.0, 1.0);
}

void main() {
    // Fix inverted Y caused by the 90-degree X-axis rotation in the transform
    vec2 uv = vec2(pos2d.x, -pos2d.y);
    float fuelPct = clamp(u_fuelLevel / 100.0, 0.0, 1.0);

    float dist = length(uv);
    // Angle: 0 at top, negative = left, positive = right
    float angle = atan(uv.x, uv.y);

    // Arc sweep: ~264 degrees total (E on left, F on right)
    float arcStart = -2.3;
    float arcEnd = 2.3;
    float arcSpan = arcEnd - arcStart;

    // --- Gauge Face ---
    float face = smoothstep(0.96, 0.92, dist);
    vec3 col = vec3(0.06);

    // --- Outer Bezel ---
    float bezel = smoothstep(0.96, 0.93, dist) - smoothstep(0.90, 0.87, dist);
    col = mix(col, vec3(0.2), bezel);

    // --- Major Tick Marks (0%, 25%, 50%, 75%, 100%) ---
    float ticks = 0.0;
    for (int i = 0; i <= 4; i++) {
        float t = float(i) / 4.0;
        float ta = arcStart + t * arcSpan;
        ticks += smoothstep(0.03, 0.012, abs(angle - ta))
               * smoothstep(0.58, 0.62, dist)
               * smoothstep(0.87, 0.85, dist);
    }

    // --- Minor Tick Marks ---
    for (int i = 0; i <= 8; i++) {
        float t = float(i) / 8.0;
        float ta = arcStart + t * arcSpan;
        ticks += smoothstep(0.014, 0.006, abs(angle - ta))
               * smoothstep(0.70, 0.73, dist)
               * smoothstep(0.87, 0.85, dist);
    }
    col = mix(col, vec3(0.85), clamp(ticks, 0.0, 1.0));

    // --- Colored Fill Arc ---
    float needleAngle = arcStart + fuelPct * arcSpan;
    float inArc = step(arcStart, angle) * step(angle, arcEnd);
    float arcBand = smoothstep(0.86, 0.84, dist) - smoothstep(0.79, 0.77, dist);
    float fillMask = step(angle, needleAngle) * inArc * arcBand;

    // Red -> yellow -> green gradient
    float normPos = clamp((angle - arcStart) / arcSpan, 0.0, 1.0);
    vec3 arcColor;
    if (normPos < 0.3) {
        arcColor = mix(vec3(0.9, 0.1, 0.0), vec3(1.0, 0.7, 0.0), normPos / 0.3);
    } else {
        arcColor = mix(vec3(1.0, 0.7, 0.0), vec3(0.1, 0.85, 0.2), (normPos - 0.3) / 0.7);
    }
    col = mix(col, arcColor, fillMask);

    // Dim outline for unfilled portion
    float emptyMask = step(needleAngle, angle) * inArc * arcBand;
    col = mix(col, vec3(0.14), emptyMask);

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

    // --- E and F Labels ---
    float eMark = letterE(uv, vec2(-0.38, 0.30), 0.7);
    float fMark = letterF(uv, vec2(0.32, 0.30), 0.7);
    col = mix(col, vec3(0.85), clamp(eMark + fMark, 0.0, 1.0));

    // --- Low Fuel Warning ---
    float alpha = face;
    if (fuelPct < u_lowFuelWarning) {
        float pulse = 0.55 + 0.45 * sin(u_time * 6.0);
        col = mix(col, vec3(0.8, 0.0, 0.0), 0.15 * (1.0 - pulse));
        alpha *= (0.7 + 0.3 * pulse);
    }

    color = vec4(col, alpha);
}
