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

out Vector {
	vec3 color;
	vec2 uv;
} OUT;

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 texCoords;

layout(location = 1) uniform vec2 simulationBounds = vec2(1000.0, 1000.0);
layout(location = 2) uniform vec2 screenBounds = vec2(500.0, 500.0);
layout(location = 5) uniform float atomDiameter = 3.0;

void main() {
	Atom atom = atoms[gl_InstanceID];
	gl_Position = vec4(position.xy * max(atomDiameter * screenBounds / simulationBounds, vec2(6.0)) / screenBounds + vec2(atom.x, atom.y) * 2.0 / simulationBounds - vec2(1.0), 0.0, 1.0);
	AtomType at = atomTypes[atom.atomType];
	OUT.color = vec3(at.r, at.g, at.b);
	OUT.uv = position.xy;
}