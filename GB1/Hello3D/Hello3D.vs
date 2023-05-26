#version 400

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 texCoord;
layout (location = 3) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 finalColor;
out vec3 scaledNormal;
out vec3 fragmentPos;

void main() {
	gl_Position = projection * view * model * vec4(position, 1.0f);
	finalColor = color;
	scaledNormal = mat3(transpose(inverse(model))) * normal;
	fragmentPos = vec3(model * vec4(position, 1.0f));
}