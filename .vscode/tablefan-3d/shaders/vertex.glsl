# Vertex Shader for Table Fan 3D Model

#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragNormal;
out vec3 fragPosition;

void main()
{
    fragPosition = vec3(model * vec4(position, 1.0));
    fragNormal = mat3(transpose(inverse(model))) * normal; // Transform normal to world space
    gl_Position = projection * view * vec4(fragPosition, 1.0);
}