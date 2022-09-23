#ifndef LIFESIMULATIONC_LIFESIMULATIONHANDLER_H
#define LIFESIMULATIONC_LIFESIMULATIONHANDLER_H

#include "../model/LifeSimulationStructures.h"

#include <SDL2/SDL.h>
#include <vector>

class LifeSimulationHandler {
public:
    LifeSimulationHandler();
    ~LifeSimulationHandler();

    void setBounds(float simWidth, float simHeight);
    float getWidth() const;
    float getHeight() const;

    /**
     * Remove AtomType matching \p id and all relevant interaction values.<br>
     * Removal of AtomTypes which don't exist is safe and will have no effect.
     * @param id
     */
    void removeAtomType(uint id);

    void initSimulation();
    void iterateSimulation();

    void shuffleAtomPositions();
    void shuffleAtomInteractions();

    std::vector<Atom*>* getAtoms();
private:
    float mSimWidth = 0;
    float mSimHeight = 0;

    std::vector<Atom*> mAtoms;

//    std::unordered_map<uint, AtomType*> mAtomTypes;
    LifeSimulationRules mLSRules;
};


#endif //LIFESIMULATIONC_LIFESIMULATIONHANDLER_H
