#ifndef RENDERER_H
#define RENDERER_H

#ifdef __cplusplus
extern "C" {
#endif

// Function to initialize the renderer
void initRenderer(const char *vertexShaderPath, const char *fragmentShaderPath);

// Function to load shaders from files
unsigned int loadShader(const char *path, unsigned int type);

// Function to set up vertex buffers for the 3D model
void setupVertexBuffers(const char *modelPath);

// Function to render the 3D model
void renderModel();

// Function to clean up resources used by the renderer
void cleanupRenderer();

#ifdef __cplusplus
}
#endif

#endif // RENDERER_H