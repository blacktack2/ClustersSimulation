#ifndef LIFESIMULATIONC_LIFESIMULATIONHANDLER_H
#define LIFESIMULATIONC_LIFESIMULATIONHANDLER_H

#include "GLUtilities.h"
#include "../model/SimulationStructures.h"
#ifdef ITERATE_ON_COMPUTE_SHADER
#include "ComputeShader.h"

#include "glad/glad.h"
#include <array>
#else
#include <vector>
#endif


#ifdef ITERATE_ON_COMPUTE_SHADER
#define ATOMS_BUFFER_SIZE 10000
#define ATOM_TYPES_BUFFER_SIZE 50
#define INTERACTIONS_BUFFER_SIZE ATOM_TYPES_BUFFER_SIZE * ATOM_TYPES_BUFFER_SIZE

#define INTERACTION_INDEX(aId, bId) aId == bId ? aId * aId : (aId < bId ? bId * bId + aId * 2 + 1 : aId * aId + bId * 2 + 2)
#endif

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
    [[nodiscard]] inline const float& getDt() const { return mDt; }

    void setDrag(float drag);
    [[nodiscard]] inline const float& getDrag() const { return mDrag; }

    void setInteractionRange(float interactionRange);
    [[nodiscard]] inline const float& getInteractionRange() const { return mInteractionRange; }

    void setCollisionForce(float collisionForce);
    [[nodiscard]] inline const float& getCollisionForce() const { return mCollisionForce; }

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

#ifndef ITERATE_ON_COMPUTE_SHADER
    const std::vector<Atom>& getAtoms();
#endif
private:
    float mSimWidth;
    float mSimHeight;

    float mDt;
    float mDrag;
    float mInteractionRange;
    float mInteractionRange2;
    float mCollisionForce;

#ifdef ITERATE_ON_COMPUTE_SHADER
    ComputeShader mIterationComputePass1;
    ComputeShader mIterationComputePass2;

    GLuint mAtomTypesBufferID;
    GLuint mAtomsBufferID;
    GLuint mInteractionsBufferID;

    size_t mAtomTypeCount;
    size_t mAtomCount;
    size_t mInteractionCount;

    std::array<AtomType, ATOM_TYPES_BUFFER_SIZE> mAtomTypes;
    std::array<AtomTypeRaw, ATOM_TYPES_BUFFER_SIZE> mAtomTypesBuffer;
    std::array<Atom, ATOMS_BUFFER_SIZE> mAtomsBuffer;
    std::array<float, INTERACTIONS_BUFFER_SIZE> mInteractionsBuffer;

    const std::string SIMULATION_BOUNDS_UNIFORM = "simulationBounds";
    const std::string INTERACTION_RANGE2_UNIFORM = "interactionRange2";
    const std::string ATOM_DIAMETER_UNIFORM = "atomDiameter";
    const std::string COLLISION_FORCE_UNIFORM = "collisionForce";
    const std::string DRAG_FORCE_UNIFORM = "dragForce";
    const std::string DT_UNIFORM = "dt";
#else
    std::vector<Atom> mAtoms;
    SimulationRules mLSRules;
#endif
};
#endif //LIFESIMULATIONC_LIFESIMULATIONHANDLER_H
