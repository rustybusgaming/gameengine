#version 450
// Vulkan GLSL Fragment Shader for Simple Triangle
// Nexus Engine - Triangle Demo

// Input from vertex shader
layout(location = 0) in vec3 fragColor;

// Output color
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
