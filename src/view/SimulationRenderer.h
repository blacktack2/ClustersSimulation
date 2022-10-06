#ifndef LIFESIMULATIONC_LIFESIMULATIONRENDERER_H
#define LIFESIMULATIONC_LIFESIMULATIONRENDERER_H

#include "../control/SimulationHandler.h"

#include <cstdio>

class SimulationRenderer {
public:
    explicit SimulationRenderer(SimulationHandler& handler);
    ~SimulationRenderer();

    void drawSimulation(float startX, float startY, float width, float height);
private:
    SimulationHandler& mHandler;
};


#endif //LIFESIMULATIONC_LIFESIMULATIONRENDERER_H
