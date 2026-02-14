#ifdef GL_ES
#version 300 es
precision highp float;
#else
#version 330 core
#endif

in vec2 pos2d;
uniform float u_health; // 0.0 to 1.0
uniform float u_time;

out vec4 color;

// Signed distance for a rounded box
float sdRoundBox(vec2 p, vec2 b, float r) {
    vec2 d = abs(p) - b;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0) - r;
}

// Signed distance for a symmetric trapezoid
// r1 = bottom half-width, r2 = top half-width, he = half-height
float sdTrapezoid(vec2 p, float r1, float r2, float he) {
    vec2 k1 = vec2(r2, he);
    vec2 k2 = vec2(r2 - r1, 2.0 * he);
    p.x = abs(p.x);
    vec2 ca = vec2(p.x - min(p.x, (p.y < 0.0) ? r1 : r2), abs(p.y) - he);
    vec2 cb = p - k1 + k2 * clamp(dot(k1 - p, k2) / dot(k2, k2), 0.0, 1.0);
    float s = (cb.x < 0.0 && ca.y < 0.0) ? -1.0 : 1.0;
    return s * sqrt(min(dot(ca, ca), dot(cb, cb)));
}

float planeSDF(vec2 p) {
    float d = 1e10;

    // Fuselage
    d = min(d, sdRoundBox(p, vec2(0.05, 0.32), 0.02));

    // Nose cone
    d = min(d, sdTrapezoid(p - vec2(0.0, 0.40), 0.06, 0.015, 0.14));

    // Main wings (swept, symmetric via abs)
    vec2 wp = vec2(abs(p.x) - 0.05, p.y + 0.02);
    wp.y += wp.x * 0.18;
    d = min(d, sdRoundBox(wp, vec2(0.33, 0.04), 0.01));

    // Tail horizontal stabilizers
    vec2 tp = vec2(abs(p.x) - 0.04, p.y + 0.32);
    tp.y += tp.x * 0.35;
    d = min(d, sdRoundBox(tp, vec2(0.15, 0.022), 0.008));

    // Vertical stabilizer
    d = min(d, sdRoundBox(p - vec2(0.0, -0.38), vec2(0.012, 0.06), 0.005));

    return d;
}

void main() {
    // Fix inverted Y caused by the 90-degree X-axis rotation in the transform
    vec2 uv = vec2(pos2d.x, -pos2d.y);
    float health = clamp(u_health, 0.0, 1.0);

    float d = planeSDF(uv);

    // Plane fill mask (inside the shape)
    float planeMask = smoothstep(0.01, -0.01, d);

    // Outline (thin bright edge around the shape)
    float outline = smoothstep(0.025, 0.015, abs(d)) * smoothstep(-0.005, 0.005, d + 0.02);

    // Damage spread: distance from center, weighted so wing tips go red first
    float extremity = length(uv * vec2(1.6, 0.9));
    float maxExtremity = 0.85;
    float normDist = clamp(extremity / maxExtremity, 0.0, 1.0);

    // A pixel is damaged when its normalized distance exceeds the health level
    // health=1.0 -> nothing red, health=0.0 -> everything red
    float isDamaged = smoothstep(health - 0.08, health + 0.08, normDist);

    // Colors
    vec3 healthyColor = vec3(0.1, 0.8, 0.25);
    vec3 damagedColor = vec3(0.9, 0.1, 0.0);
    vec3 outlineColor = vec3(0.7, 0.8, 0.9);

    vec3 fillColor = mix(healthyColor, damagedColor, isDamaged);

    // Damage boundary glow (orange line where damage meets healthy)
    float boundary = smoothstep(0.06, 0.0, abs(normDist - health)) * planeMask;
    fillColor = mix(fillColor, vec3(1.0, 0.6, 0.0), boundary * 0.5);

    // Low health pulse
    if (health < 0.25) {
        float pulse = 0.55 + 0.45 * sin(u_time * 6.0);
        fillColor = mix(fillColor, damagedColor, 0.3 * (1.0 - pulse));
        outline *= (0.7 + 0.3 * pulse);
    }

    // Compose: dark transparent background within a circle, then plane on top
    float bgCircle = smoothstep(0.95, 0.90, length(uv));
    vec3 col = vec3(0.04);
    col = mix(col, fillColor, planeMask * 0.85);
    col = mix(col, outlineColor, outline);

    float alpha = bgCircle * 0.4;
    alpha = max(alpha, planeMask * 0.8);
    alpha = max(alpha, outline);

    color = vec4(col, alpha);
}
