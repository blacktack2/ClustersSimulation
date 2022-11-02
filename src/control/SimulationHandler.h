#pragma once
#include "GLUtilities.h"
#include "../model/SimulationStructures.h"
#ifdef ITERATE_ON_COMPUTE_SHADER
#include "ComputeShader.h"

#include "glad/glad.h"
#endif

#include <array>

#ifdef ITERATE_ON_COMPUTE_SHADER
const unsigned int MAX_ATOMS = 10000;
#else
const unsigned int MAX_ATOMS = 3000;
#endif
const unsigned int MAX_ATOM_TYPES = 50;
const unsigned int MAX_INTERACTIONS = MAX_ATOM_TYPES * MAX_ATOM_TYPES;

#define INTERACTION_INDEX(aId, bId) (aId == bId ? aId * aId : (aId < bId ? bId * bId + aId * 2 + 1 : aId * aId + bId * 2 + 2))

class SimulationHandler {
public:
    SimulationHandler();
    ~SimulationHandler();

#ifdef ITERATE_ON_COMPUTE_SHADER
    void initComputeShaders();
#endif

    void setBounds(float simWidth, float simHeight);
    [[nodiscard]] float getWidth() const;
    [[nodiscard]] float getHeight() const;

    void setDt(float dt);
    [[nodiscard]] inline float getDt() const { return mDt; }

    void setDrag(float drag);
    [[nodiscard]] inline float getDrag() const { return mDrag; }

    void setInteractionRange(float interactionRange);
    [[nodiscard]] inline float getInteractionRange() const { return mInteractionRange; }

    void setCollisionForce(float collisionForce);
    [[nodiscard]] inline float getCollisionForce() const { return mCollisionForce; }

    void setAtomDiameter(float atomDiameter);
    [[nodiscard]] inline float getAtomDiameter() const { return mAtomDiameter; }

    void clearAtoms();
    void initSimulation();
    void iterateSimulation();

    unsigned int newAtomType();
    void removeAtomType(unsigned int atomTypeId);
    void clearAtomTypes();

    [[nodiscard]] std::vector<unsigned int> getAtomTypeIds() const;
    void setAtomTypeColor(unsigned int atomTypeId, Color color);
    void setAtomTypeColorR(unsigned int atomTypeId, float r);
    void setAtomTypeColorG(unsigned int atomTypeId, float g);
    void setAtomTypeColorB(unsigned int atomTypeId, float b);
    [[nodiscard]] Color getAtomTypeColor(unsigned int atomTypeId) const;
    void setAtomTypeQuantity(unsigned int atomTypeId, unsigned int quantity);
    [[nodiscard]] unsigned int getAtomTypeQuantity(unsigned int atomTypeId) const;
    void setAtomTypeFriendlyName(unsigned int atomTypeId, const std::string& friendlyName);
    [[nodiscard]] std::string getAtomTypeFriendlyName(unsigned int atomTypeId) const;

    [[nodiscard]] float getInteraction(unsigned int aId, unsigned int bId) const;
    void setInteraction(unsigned int aId, unsigned int bId, float value);

    void shuffleAtomPositions();
    void shuffleAtomInteractions();
    void zeroAtomInteractions();

    [[nodiscard]] unsigned int getAtomCount() const;
    [[nodiscard]] unsigned int getActualAtomCount() const;
    [[nodiscard]] unsigned int getAtomTypeCount() const;

    const std::array<Atom, MAX_ATOMS>& getAtoms();
private:
    float mSimWidth;
    float mSimHeight;

    float mDt;
    float mDrag;
    float mInteractionRange;
    float mInteractionRange2;
    float mCollisionForce;
    float mAtomDiameter;

    size_t mAtomTypeCount;
    size_t mAtomCount;
    size_t mInteractionCount;

    std::array<AtomType, MAX_ATOM_TYPES> mAtomTypes;
    std::array<AtomTypeRaw, MAX_ATOM_TYPES> mAtomTypesBuffer;
    std::array<Atom, MAX_ATOMS> mAtomsBuffer;
    std::array<float, MAX_INTERACTIONS> mInteractionsBuffer;

#ifdef ITERATE_ON_COMPUTE_SHADER
    ComputeShader mIterationComputePass1;
    ComputeShader mIterationComputePass2;

    GLuint mAtomTypesBufferID;
    GLuint mAtomsBufferID;
    GLuint mInteractionsBufferID;

    const std::string SIMULATION_BOUNDS_UNIFORM = "simulationBounds";
    const std::string INTERACTION_RANGE2_UNIFORM = "interactionRange2";
    const std::string ATOM_DIAMETER_UNIFORM = "atomDiameter";
    const std::string COLLISION_FORCE_UNIFORM = "collisionForce";
    const std::string DRAG_FORCE_UNIFORM = "dragForce";
    const std::string DT_UNIFORM = "dt";
#endif
};
