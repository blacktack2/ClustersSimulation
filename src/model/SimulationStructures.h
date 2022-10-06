#ifndef CLUSTERSSIMULATION_LIFESIMULATIONSTRUCTURES_H
#define CLUSTERSSIMULATION_LIFESIMULATIONSTRUCTURES_H

#include <string>
#include <vector>
#include <optional>
#include <memory>

struct Color {
    float r;
    float g;
    float b;
};

class Atom {
public:
    Atom();
    Atom(const Atom& other);

    float mX;
    float mY;

    float mVX;
    float mVY;

    float mFX;
    float mFY;
};

class AtomType {
public:
    AtomType();
    AtomType(unsigned int id);
    AtomType(const AtomType& other);
    ~AtomType();
    AtomType& operator=(AtomType&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        mColor = other.mColor;
        mQuantity = other.mQuantity;
        mFriendlyName = other.mFriendlyName;
        mAtoms = other.mAtoms;

        return *this;
    }

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

    Atom& newAtom();
    std::vector<Atom>& getAtoms();
    void clearAtoms();
private:
    const unsigned int mId;

    Color mColor;
    unsigned int mQuantity;

    std::string mFriendlyName;

    std::vector<Atom> mAtoms;
};

struct InteractionSet {
    unsigned int aId;
    unsigned int bId;
    float value;
};

class SimulationRules {
public:
    SimulationRules();
    ~SimulationRules();

    void clearAtomTypes();

    AtomType& newAtomType();
    AtomType& newAtomType(unsigned int id);
    std::optional<std::reference_wrapper<AtomType>> getAtomType(unsigned int atomTypeId);
    void removeAtomType(unsigned int atomTypeId);

    std::vector<AtomType>& getAtomTypes();

    unsigned int getAtomCount();

    void clearInteractions();
    void setInteraction(unsigned int aId, unsigned int bId, float interaction);
    float getInteraction(unsigned int aId, unsigned int bId);
    [[nodiscard]] std::vector<InteractionSet>& getInteractions();

    void setAtomRadius(float atomRadius);
    float getAtomRadius();
private:
    void createInteractionsForAtomType(AtomType& atomType);

    float mAtomRadius;

    std::vector<AtomType> mAtomTypes;
    std::vector<InteractionSet> mInteractions;
};

Color hslToColor(float h, float s, float l);

#endif //CLUSTERSSIMULATION_LIFESIMULATIONSTRUCTURES_H
