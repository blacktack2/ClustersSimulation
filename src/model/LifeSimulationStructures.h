#ifndef CLUSTERSSIMULATION_LIFESIMULATIONSTRUCTURES_H
#define CLUSTERSSIMULATION_LIFESIMULATIONSTRUCTURES_H

#include <string>
#include <vector>

struct Color {
    float r;
    float g;
    float b;
};

class AtomType {
public:
    AtomType();
    ~AtomType();

    [[nodiscard]] unsigned int getId() const;

    Color getColor();
    void setColor(Color color);

    void setColorR(float r);
    void setColorG(float g);
    void setColorB(float b);

    [[nodiscard]] unsigned int getQuantity() const;
    void setQuantity(unsigned int quantity);

    std::string getFriendlyName();
    void setFriendlyName(std::string friendlyName);
private:
    const unsigned int mId;

    Color mColor;
    unsigned int mQuantity;

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
    unsigned int aId;
    unsigned int bId;
    float value;
};

class LifeSimulationRules {
public:
    LifeSimulationRules();
    ~LifeSimulationRules();

    void clear();

    AtomType* newAtomType();
    AtomType* addAtomType(AtomType* atomType);
    AtomType* getAtomType(unsigned int atomTypeId);
    void removeAtomType(unsigned int atomTypeId);

    std::vector<AtomType*>& getAtomTypes();

    void setInteraction(unsigned int aId, unsigned int bId, float interaction);
    float getInteraction(unsigned int aId, unsigned int bId);
private:
    Color hslToColor(float h, float s, float l);

    std::vector<AtomType*> mAtomTypes;
    std::vector<InteractionSet> mInteractions;
};

#endif //CLUSTERSSIMULATION_LIFESIMULATIONSTRUCTURES_H
