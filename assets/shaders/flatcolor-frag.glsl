#ifdef GL_ES
#version 300 es
precision highp float;
#else
#version 330 core
#endif

layout (std140) uniform GlobalVals {
	float viewdist;
};

out vec4 color;
in float lighting;
in vec3 normal;
in vec3 fragpos;

uniform vec3 camerapos;
uniform vec3 lightdir;
uniform float specularfactor;
uniform vec3 basecolor;

const float FOG_DIST = 10000.0;

void main()
{
	float d = length(fragpos - camerapos);

	color = vec4(basecolor, 1.0);
	color *= lighting;
	color.a = 1.0;

	vec3 reflected = normalize(reflect(lightdir, normal));
	float spec = pow(dot(reflected, normalize(camerapos - fragpos)), 16.0) * specularfactor;
	color += vec4(1.0, 1.0, 1.0, 0.0) * spec;
	color = clamp(color, 0.0, 1.0);

	vec4 fogeffect = mix(color, vec4(0.5, 0.8, 1.0, 1.0), min(max(0.0, d - viewdist) / FOG_DIST, 1.0));
	color = fogeffect;
}
