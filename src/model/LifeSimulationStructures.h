#ifndef CLUSTERSSIMULATION_LIFESIMULATIONSTRUCTURES_H
#define CLUSTERSSIMULATION_LIFESIMULATIONSTRUCTURES_H

#include <SDL2/SDL.h>
#include <map>
#include <vector>
#include <unordered_map>

struct Color {
    float r;
    float g;
    float b;
};

class AtomType {
public:
    AtomType();
    ~AtomType();

    [[nodiscard]] uint getId() const;

    Color getColor();
    void setColor(Color color);

    void setColorR(float r);
    void setColorG(float g);
    void setColorB(float b);

    [[nodiscard]] uint getQuantity() const;
    void setQuantity(uint quantity);

    std::string getFriendlyName();
    void setFriendlyName(std::string friendlyName);
private:
    const uint mId;

    Color mColor;
    uint mQuantity;

    std::string mFriendlyName;
};

class Atom {
public:
    explicit Atom(AtomType* atomType);

    AtomType* getAtomType();

    float mX;
    float mY;

    float mVX;
    float mVY;
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

    void newAtomType();
    void addAtomType(AtomType* atomType);
    AtomType* getAtomType(uint atomTypeId);
    void removeAtomType(uint atomTypeId);

    std::vector<AtomType*>* getAtomTypes();

    void setInteraction(uint aId, uint bId, float interaction);
    float getInteraction(uint aId, uint bId);
private:
    Color hslToColor(float h, float s, float l);

    std::vector<AtomType*> mAtomTypes;
    std::vector<InteractionSet> mInteractions;
};

#endif //CLUSTERSSIMULATION_LIFESIMULATIONSTRUCTURES_H
