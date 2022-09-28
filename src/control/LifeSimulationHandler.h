#ifndef LIFESIMULATIONC_LIFESIMULATIONHANDLER_H
#define LIFESIMULATIONC_LIFESIMULATIONHANDLER_H

#include "../model/LifeSimulationStructures.h"

#include <vector>

class LifeSimulationHandler {
public:
    LifeSimulationHandler();
    ~LifeSimulationHandler();

    void setBounds(float simWidth, float simHeight);
    [[nodiscard]] float getWidth() const;
    [[nodiscard]] float getHeight() const;

    void clearSimulation();
    void initSimulation();
    void iterateSimulation();

    void shuffleAtomPositions();
    void shuffleAtomInteractions();

    std::vector<Atom*>* getAtoms();
    LifeSimulationRules* getLSRules();
private:
    float mSimWidth;
    float mSimHeight;

    std::vector<Atom*> mAtoms;
    LifeSimulationRules mLSRules;
};


#endif //LIFESIMULATIONC_LIFESIMULATIONHANDLER_H
