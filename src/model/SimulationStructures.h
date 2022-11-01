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

#ifdef ITERATE_ON_COMPUTE_SHADER
struct vec2 { float x; float y; };

struct AtomType {
    AtomType();
    explicit AtomType(unsigned int id);
    unsigned int id;

    float r;
    float g;
    float b;

    unsigned int quantity;
    std::string friendlyName;
};

struct AtomTypeRaw {
    explicit AtomTypeRaw();
    explicit AtomTypeRaw(const AtomType& at);
    float r;
    float g;
    float b;
};

struct Atom {
    Atom();
    explicit Atom(unsigned int atomType);

    float x, y, vx, vy, fx, fy;

    unsigned int atomType;
};
#else

class AtomType {
public:
    AtomType();
    explicit AtomType(unsigned int id);
    AtomType(const AtomType& other);
    ~AtomType();

    [[nodiscard]] const unsigned int & getId() const;

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
    unsigned int mId;

    Color mColor;
    unsigned int mQuantity;

    std::string mFriendlyName;
};

struct Atom {
    explicit Atom(AtomType* atomType);

    AtomType* mAtomType;

    float mX;
    float mY;

    float mVX;
    float mVY;

    float mFX;
    float mFY;
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

    AtomType* newAtomType();
    AtomType* newAtomType(unsigned int id);
    AtomType* getAtomType(unsigned int atomTypeId) const;
    void removeAtomType(unsigned int atomTypeId);

    const std::vector<std::unique_ptr<AtomType>>& getAtomTypes() const;

    [[nodiscard]] int getAtomCount() const;
    [[nodiscard]] unsigned int getAtomTypeCount() const;

    void clearInteractions();
    void setInteraction(unsigned int aId, unsigned int bId, float interaction);
    float getInteraction(unsigned int aId, unsigned int bId) const;
    [[nodiscard]] std::vector<InteractionSet>& getInteractions();

    void setAtomRadius(float atomRadius);
    [[nodiscard]] const float& getAtomRadius() const;
private:
    void createInteractionsForAtomType(AtomType* atomType);

    float mAtomRadius;

    std::vector<std::unique_ptr<AtomType>> mAtomTypes;
    std::vector<InteractionSet> mInteractions;
};
#endif

Color hslToColor(float h, float s, float l);

#endif //CLUSTERSSIMULATION_LIFESIMULATIONSTRUCTURES_H
