R"(
#version 460 core

struct AtomType {
	float r;
	float g;
	float b;
};

layout(std430, binding = 1) buffer AtomTypeBuffer {
	AtomType atomTypes[50];
};

struct Atom {
    float x, y, vx, vy, fx, fy;
	uint atomType;
};

layout(std430, binding = 2) buffer AtomBuffer {
	Atom atoms[10000];
};

in Vector {
	vec3 color;
	vec2 uv;
} IN;

layout(binding = 0) uniform sampler2D inTexture;

layout(location = 0) out vec4 fragColor;

void main() {
	float dist = dot(IN.uv, IN.uv);
	fragColor = vec4(IN.color, clamp(1.0 - dist * dist, 0.0, 1.0));
}
)";