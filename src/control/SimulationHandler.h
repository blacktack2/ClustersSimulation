#ifndef LIFESIMULATIONC_LIFESIMULATIONHANDLER_H
#define LIFESIMULATIONC_LIFESIMULATIONHANDLER_H

//#define ITERATE_ON_COMPUTE_SHADER

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

    //void updateRenderTexture(int width, int height);
#endif

    void setBounds(float simWidth, float simHeight);
    [[nodiscard]] float getWidth() const;
    [[nodiscard]] float getHeight() const;

    void setDt(float dt);
    float getDt();

    void setDrag(float drag);
    float getDrag();

    void setInteractionRange(float interactionRange);
    float getInteractionRange();

    void setCollisionForce(float collisionForce);
    float getCollisionForce();

    void clearAtoms();
    void initSimulation();
    void iterateSimulation();

    unsigned int newAtomType();
    //void newAtomType(unsigned int id);
    void removeAtomType(unsigned int atomTypeId);
    void clearAtomTypes();

    std::vector<unsigned int> getAtomTypeIds();
    void setAtomTypeColor(unsigned int atomTypeId, Color color);
    void setAtomTypeColorR(unsigned int atomTypeId, float r);
    void setAtomTypeColorG(unsigned int atomTypeId, float g);
    void setAtomTypeColorB(unsigned int atomTypeId, float b);
    Color getAtomTypeColor(unsigned int atomTypeId);
    void setAtomTypeQuantity(unsigned int atomTypeId, unsigned int quantity);
    unsigned int getAtomTypeQuantity(unsigned int atomTypeId);
    void setAtomTypeFriendlyName(unsigned int atomTypeId, std::string friendlyName);
    std::string getAtomTypeFriendlyName(unsigned int atomTypeId);

    float getInteraction(unsigned int aId, unsigned int bId);
    void setInteraction(unsigned int aId, unsigned int bId, float value);

    void shuffleAtomPositions();
    void shuffleAtomInteractions();
    void zeroAtomInteractions();

    unsigned int getAtomCount();

    //GLuint getRenderTexture();

#ifdef ITERATE_ON_COMPUTE_SHADER
    std::array<Atom, ATOMS_BUFFER_SIZE>& getAtoms();
#else
    std::vector<Atom>& getAtoms();
#endif
private:
#ifdef ITERATE_ON_COMPUTE_SHADER
    //void newRenderTexture(int width, int height);
    //void simulationRenderPass();

    //void readAtomsBuffer();
    //void writeAtomsBuffer();
    //void writeAtomTypesBuffer();
    //void writeInteractionsBuffer();
#endif
    float mSimWidth;
    float mSimHeight;

    float mDt;
    float mDrag;
    float mInteractionRange;
    float mInteractionRange2;
    float mCollisionForce;

#ifdef ITERATE_ON_COMPUTE_SHADER

    ComputeShader mIterationComputeShader;

    //GLuint mComputeShaderPass1;
    //GLuint mComputeShaderPass2;
    //GLuint mComputeShaderPass3;
    //GLuint mComputeShaderRenderPass;

    //GLuint mSimulationShaderPass1Program;
    //GLuint mSimulationShaderPass2Program;
    //GLuint mSimulationShaderPass3Program;
    //GLuint mSimulationShaderRenderPassProgram;

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

    //GLuint mRenderTexture;
#else
    std::vector<Atom> mAtoms;
    SimulationRules mLSRules;
#endif
};


#endif //LIFESIMULATIONC_LIFESIMULATIONHANDLER_H
