/**
 * @file   SimulationHandler.h
 * @brief  Handler class for running the simulation
 * 
 * @author Stuart Lewis
 * @date   January 2023
 */
#pragma once
#include "GLUtilities.h"
#include "../model/SimulationStructures.h"
#ifdef ITERATE_ON_COMPUTE_SHADER
#include "ComputeShader.h"

#include "glad/glad.h"
#endif

#include <array>

#ifdef ITERATE_ON_COMPUTE_SHADER
const size_t MAX_ATOMS = 10000;
#else
const size_t MAX_ATOMS = 3000;
#endif
const size_t MAX_ATOM_TYPES = 50;
const size_t MAX_INTERACTIONS = MAX_ATOM_TYPES * MAX_ATOM_TYPES;

const float MIN_SIM_WIDTH = 10.0f;
const float MAX_SIM_WIDTH = 1000000.0f;

const float MIN_SIM_HEIGHT = 10.0f;
const float MAX_SIM_HEIGHT = 1000000.0f;

const float MIN_DT = 0.01f;
const float MAX_DT = 10.0f;

const float MIN_DRAG = 0.0f;
const float MAX_DRAG = 1.0f;

const float MIN_INTERACTION_RANGE = 1.0f;

const float MIN_COLLISION_FORCE = 0.0f;
const float MAX_COLLISION_FORCE = 10.0f;

const float MIN_ATOM_DIAMETER = 1.0f;

const float MIN_INTERACTION = -1.0f;
const float MAX_INTERACTION = 1.0f;

#define INTERACTION_INDEX(aId, bId) (aId == bId ? aId * aId : (aId < bId ? bId * bId + aId * 2 + 1 : aId * aId + bId * 2 + 2))

/** Defines the initial positioning of the Atoms. */
enum StartCondition {
    StartConditionRandom,            /** Assign completely random positions to each Atom. */
    StartConditionEquidistant,       /** Position Atoms equadistantly, in AtomType order. */
    StartConditionRandomEquidistant, /** Position Atoms equadistantly, but in shuffled order. */
    StartConditionRings,             /** Position Atoms in concentric rings. */
    StartConditionMax                /** Max value used for array indexing. */
};

/**
 * Handler class for running the simulation.
 */
class SimulationHandler {
public:
    SimulationHandler();
    ~SimulationHandler();

#ifdef ITERATE_ON_COMPUTE_SHADER
    /**
     * Initialize OpenGL buffers and shaders.
     */
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

    atom_type_id newAtomType();
    void removeAtomType(atom_type_id atomTypeId);
    void clearAtomTypes();

    [[nodiscard]] std::vector<atom_type_id> getAtomTypeIds() const;
    void setAtomTypeColor(atom_type_id atomTypeId, glm::vec3 color);
    void setAtomTypeColorR(atom_type_id atomTypeId, float r);
    void setAtomTypeColorG(atom_type_id atomTypeId, float g);
    void setAtomTypeColorB(atom_type_id atomTypeId, float b);
    [[nodiscard]] glm::vec3 getAtomTypeColor(atom_type_id atomTypeId) const;
    void setAtomTypeQuantity(atom_type_id atomTypeId, unsigned int quantity);
    [[nodiscard]] unsigned int getAtomTypeQuantity(atom_type_id atomTypeId) const;
    void setAtomTypeFriendlyName(atom_type_id atomTypeId, const std::string& friendlyName);
    [[nodiscard]] std::string getAtomTypeFriendlyName(atom_type_id atomTypeId) const;

    [[nodiscard]] float getInteraction(atom_type_id aId, atom_type_id bId) const;
    void setInteraction(atom_type_id aId, atom_type_id bId, float value);

    void shuffleAtomInteractions();
    void zeroAtomInteractions();

    [[nodiscard]] size_t getAtomCount() const;
    [[nodiscard]] size_t getActualAtomCount() const;
    [[nodiscard]] size_t getAtomTypeCount() const;

    const std::array<Atom, MAX_ATOMS>& getAtoms();

    StartCondition startCondition;
private:
    void initAtomPositionsRandom();
    void initAtomPositionsEquidistant();
    void initAtomPositionsRandomEquidistant();
    void initAtomPositionsRings();

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
