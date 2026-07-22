#version 450
// Vulkan GLSL Vertex Shader for Simple Triangle
// Nexus Engine - Triangle Demo

// Input vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

// Output to fragment shader
layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(inPosition, 1.0);
    fragColor = inColor;
}
