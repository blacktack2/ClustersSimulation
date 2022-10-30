#ifndef LIFESIMULATIONC_LIFESIMULATIONRENDERER_H
#define LIFESIMULATIONC_LIFESIMULATIONRENDERER_H

#include "Shader.h"
#include "../control/SimulationHandler.h"

class SimulationRenderer {
public:
    explicit SimulationRenderer(SimulationHandler& handler);
    ~SimulationRenderer();

    bool init();

    void drawSimulation([[maybe_unused]] float startX, [[maybe_unused]] float startY, float width, float height);
private:
    SimulationHandler& mHandler;
#ifdef ITERATE_ON_COMPUTE_SHADER
    Shader mShader;
    GLuint mFrameBuffer;
    GLuint mTexture;

    float imageWidth = 500.0f, imageHeight = 500.0f;
#endif
};


#endif //LIFESIMULATIONC_LIFESIMULATIONRENDERER_H
