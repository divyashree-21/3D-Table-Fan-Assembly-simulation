#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "renderer.h"
#include "math_utils.h"

static GLuint shaderProgram;
static GLuint VAO;

void loadShaders(const char *vertexPath, const char *fragmentPath) {
    // Load and compile vertex and fragment shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Read vertex shader code
    char *vertexCode = readShaderFile(vertexPath);
    glShaderSource(vertexShader, 1, (const char **)&vertexCode, NULL);
    glCompileShader(vertexShader);
    checkShaderCompileErrors(vertexShader, "VERTEX");

    // Read fragment shader code
    char *fragmentCode = readShaderFile(fragmentPath);
    glShaderSource(fragmentShader, 1, (const char **)&fragmentCode, NULL);
    glCompileShader(fragmentShader);
    checkShaderCompileErrors(fragmentShader, "FRAGMENT");

    // Link shaders to create a shader program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Clean up shaders as they're linked into the program now
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void initRenderer(const char *vertexShaderPath, const char *fragmentShaderPath) {
    loadShaders(vertexShaderPath, fragmentShaderPath);

    // Set up vertex data (and buffer(s)) and configure vertex attributes
    float vertices[] = {
        // Define vertices for the table fan model
    };

    GLuint VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void renderModel() {
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36); // Adjust based on the number of vertices
    glBindVertexArray(0);
}

void cleanupRenderer() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);
}