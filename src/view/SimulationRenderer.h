/**
 * @file   SimulationRenderer.h
 * @brief  Wrapper for drawing the contents of a SimulationHandler to an ImGui image.
 * 
 * @author Stuart Lewis
 * @date   January 2023
 */
#pragma once
#include "Shader.h"
#include "../control/SimulationHandler.h"
#ifdef ITERATE_ON_COMPUTE_SHADER
#include "../model/Mesh.h"
#endif

/**
 * Wrapper class for drawing the contents of a SimulationHandler to an ImGui image.
 */
class SimulationRenderer {
public:
    explicit SimulationRenderer(SimulationHandler& handler);
    ~SimulationRenderer();

    void updateParameters();

    /**
     * Initialize the renderer. Must be called before
     * SimulationRenderer::drawSimulation. If ITERATE_ON_COMPUTE_SHADER is
     * defined, this must be called after initializing OpenGL.
     */
    bool init();

    /**
     * Draw the simulation to an ImGui image.
     * @param startX Position of the left side of the image in ImGui.
     * @param startY Position of the top of the image in ImGui.
     * @param width Width of the image in the window.
     * @param height Height of the image in the window.
     */
    void drawSimulation([[maybe_unused]] float startX, [[maybe_unused]] float startY, float width, float height);
private:
    SimulationHandler& mHandler;
#ifdef ITERATE_ON_COMPUTE_SHADER
    Shader mShader;
    GLuint mFrameBuffer;
    GLuint mTexture;

    Mesh* mQuad;

    float imageWidth = 500.0f, imageHeight = 500.0f;

    const std::string SCREEN_BOUNDS_UNIFORM = "screenBounds";
    const std::string SIMULATION_BOUNDS_UNIFORM = "simulationBounds";
    const std::string ATOM_DIAMETER_UNIFORM = "atomDiameter";
#endif
};
