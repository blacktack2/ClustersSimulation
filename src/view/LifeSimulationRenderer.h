#ifndef LIFESIMULATIONC_LIFESIMULATIONRENDERER_H
#define LIFESIMULATIONC_LIFESIMULATIONRENDERER_H

#include "../control/LifeSimulationHandler.h"

#include <SDL2/SDL.h>
#include <cstdio>

class LifeSimulationRenderer {
public:
    explicit LifeSimulationRenderer(LifeSimulationHandler* handler);
    ~LifeSimulationRenderer();

    void drawSimulation(float startX, float startY, float width, float height);
private:
    LifeSimulationHandler* mHandler;
};


#endif //LIFESIMULATIONC_LIFESIMULATIONRENDERER_H
