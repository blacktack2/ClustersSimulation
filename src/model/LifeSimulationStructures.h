#ifndef CLUSTERSSIMULATION_LIFESIMULATIONSTRUCTURES_H
#define CLUSTERSSIMULATION_LIFESIMULATIONSTRUCTURES_H

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

#endif //CLUSTERSSIMULATION_LIFESIMULATIONSTRUCTURES_H
