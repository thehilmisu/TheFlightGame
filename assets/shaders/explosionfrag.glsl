#ifdef GL_ES
#version 300 es
precision mediump float;
#else
#version 330 core
#endif

uniform sampler2D tex;
uniform float time;

in vec2 tc;
in float particleAlpha;

out vec4 color;

void main()
{
	vec4 texColor = texture(tex, tc);

	// Explosion color progression: bright yellow/white -> orange -> red -> dark (slower)
	vec3 hotColor = vec3(1.0, 1.0, 0.9);      // Bright white-yellow (early)
	vec3 fireColor = vec3(1.0, 0.5, 0.1);     // Orange-red (middle)
	vec3 smokeColor = vec3(0.3, 0.3, 0.3);    // Dark smoke (late)

	vec3 explosionColor;
	if (time < 0.4) {
		// Early: bright flash (longer flash duration)
		explosionColor = mix(hotColor, fireColor, time / 0.4);
	} else if (time < 1.0) {
		// Middle: fire to smoke (slower transition)
		float t = (time - 0.4) / 0.6;
		explosionColor = mix(fireColor, smokeColor, t);
	} else {
		// Late: dark smoke fading
		explosionColor = smokeColor;
	}

	color = texColor * vec4(explosionColor, 1.0);
	color.a *= particleAlpha;

	// Additional fade out at end (much later)
	color.a *= 1.0 - smoothstep(1.0, 1.5, time);
}
