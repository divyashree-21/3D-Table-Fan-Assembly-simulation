#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "renderer.h"
#include "tablefan.h"

// Function prototypes
void initializeGLFW();
void initializeGLEW();
void renderLoop(GLFWwindow *window);
void cleanup(GLFWwindow *window);

int main(void) {
    // Initialize GLFW
    initializeGLFW();

    // Create a windowed mode window and its OpenGL context
    GLFWwindow *window = glfwCreateWindow(800, 600, "3D Table Fan Simulation", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    initializeGLEW();

    // Initialize the renderer
    initRenderer();

    // Main rendering loop
    renderLoop(window);

    // Cleanup
    cleanup(window);
    return 0;
}

void initializeGLFW() {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }
}

void initializeGLEW() {
    glewExperimental = GL_TRUE; 
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(err));
        exit(EXIT_FAILURE);
    }
}

void renderLoop(GLFWwindow *window) {
    while (!glfwWindowShouldClose(window)) {
        // Render the 3D table fan
        renderTableFan();

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }
}

void cleanup(GLFWwindow *window) {
    glfwDestroyWindow(window);
    glfwTerminate();
}