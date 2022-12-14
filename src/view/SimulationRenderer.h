#pragma once
#include "Shader.h"
#include "../control/SimulationHandler.h"
#ifdef ITERATE_ON_COMPUTE_SHADER
#include "../model/Mesh.h"
#endif

class SimulationRenderer {
public:
    explicit SimulationRenderer(SimulationHandler& handler);
    ~SimulationRenderer();

    void updateParameters();

    bool init();

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
