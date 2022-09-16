#ifndef LIFESIMULATIONC_LIFESIMULATIONHANDLER_H
#define LIFESIMULATIONC_LIFESIMULATIONHANDLER_H

#include <SDL2/SDL.h>
#include <map>
#include <vector>
#include <unordered_map>

class AtomType {
public:
    AtomType();
    ~AtomType();

    uint getId() const;

    SDL_Color getColor();
    void setColor(SDL_Color color);

    uint getQuantity() const;
    void setQuantity(uint quantity);

    std::string getFriendlyName();
    void setFriendlyName(std::string friendlyName);

    void toString();
private:
    const uint mId;

    SDL_Color mColor{};
    uint mQuantity = 0;

    std::string mFriendlyName;
};

class Atom {
public:
    Atom(AtomType* atomType);

    AtomType* getAtomType();

    float mX = 0.0f;
    float mY = 0.0f;

    float mVX = 0.0f;
    float mVY = 0.0f;
private:
    AtomType* mAtomType;
};

struct InteractionSet {
    uint aId;
    uint bId;
    float value;
};

class LifeSimulationRules {
public:
    LifeSimulationRules();
    ~LifeSimulationRules();

    void clear();

    void addAtomType(AtomType* atomType);
    AtomType* getAtomType(uint atomTypeId);
    void removeAtomType(uint atomTypeId);

    std::vector<AtomType*>* getAtomTypes();

    void setInteraction(uint aId, uint bId, float interaction);
    float getInteraction(uint aId, uint bId);
private:
    std::vector<AtomType*> mAtomTypes;
    std::vector<InteractionSet> mInteractions;
};

class LifeSimulationHandler {
public:
    LifeSimulationHandler();
    ~LifeSimulationHandler();

    void setBounds(float simWidth, float simHeight);

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
