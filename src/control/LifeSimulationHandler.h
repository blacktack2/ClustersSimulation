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

    std::vector<Atom*>& getAtoms();
    LifeSimulationRules& getLSRules();
private:
    float mSimWidth;
    float mSimHeight;

    float mDt;
    float mDrag;

    std::vector<Atom*> mAtoms;
    LifeSimulationRules mLSRules;
};


#endif //LIFESIMULATIONC_LIFESIMULATIONHANDLER_H
