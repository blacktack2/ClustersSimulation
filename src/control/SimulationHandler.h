#ifndef LIFESIMULATIONC_LIFESIMULATIONHANDLER_H
#define LIFESIMULATIONC_LIFESIMULATIONHANDLER_H

#include "../model/SimulationStructures.h"

#include <vector>

class SimulationHandler {
public:
    SimulationHandler();
    ~SimulationHandler();

    void setBounds(float simWidth, float simHeight);
    [[nodiscard]] float getWidth() const;
    [[nodiscard]] float getHeight() const;

    void setDt(float dt);
    float getDt();

    void setDrag(float drag);
    float getDrag();

    void clearAtoms();
    void clearAtomTypes();
    void initSimulation();
    void iterateSimulation();

    void removeAtomType(unsigned int atomTypeId);
    void shuffleAtomPositions();
    void shuffleAtomInteractions();

    SimulationRules& getLSRules();
private:
    float mSimWidth;
    float mSimHeight;

    float mDt;
    float mDrag;

    SimulationRules mLSRules;
};


#endif //LIFESIMULATIONC_LIFESIMULATIONHANDLER_H
