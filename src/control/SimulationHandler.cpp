#include "SimulationHandler.h"

#include "../view/Logger.h"

#include "../../glm/vec3.hpp"

#include <algorithm>
#include <iterator>
#include <random>
#ifdef ITERATE_ON_COMPUTE_SHADER
#include <climits>
#include <iostream>

const char* SHADER_CODE_PASS1 =
#include "../shaders/IterationPass1.comp"
;
const char* SHADER_CODE_PASS2 =
#include "../shaders/IterationPass2.comp"
;
#endif

SimulationHandler::SimulationHandler() :
startCondition(StartConditionRandom),
mSimWidth(0), mSimHeight(0), mDt(1.0f), mDrag(0.5f),
mInteractionRange(80), mInteractionRange2(6400), mCollisionForce(1.0f), mAtomDiameter(3.0f)
#ifdef ITERATE_ON_COMPUTE_SHADER
, mIterationComputePass1(SHADER_CODE_PASS1), mIterationComputePass2(SHADER_CODE_PASS2),
mAtomTypesBufferID(), mAtomsBufferID(), mInteractionsBufferID()
#endif
, mAtomCount(0), mAtomTypeCount(0), mInteractionCount(0),
mAtomTypes(), mAtomTypesBuffer(), mAtomsBuffer(), mInteractionsBuffer()
{
    Logger::getLogger().logMessage("Constructing Handler");
}

SimulationHandler::~SimulationHandler() {
    Logger::getLogger().logMessage("Destroying Handler");
    clearAtoms();
}

#ifdef ITERATE_ON_COMPUTE_SHADER
void SimulationHandler::initComputeShaders() {
    Logger::getLogger().logMessage("Initializing Handler Compute Shaders");
    mIterationComputePass1.init();
    if (!mIterationComputePass1.isValid()) {
        Logger::getLogger().logError(std::string("Failed to initialize Compute Shader Pass1"));
        return;
    }
    mIterationComputePass2.init();
    if (!mIterationComputePass2.isValid()) {
        Logger::getLogger().logError(std::string("Failed to initialize Compute Shader Pass2"));
        return;
    }
    mAtomTypesBufferID    = BaseShader::createBuffer(mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer), 1);
    mAtomsBufferID        = BaseShader::createBuffer(mAtomsBuffer.data(), sizeof(mAtomsBuffer), 2);
    mInteractionsBufferID = BaseShader::createBuffer(mInteractionsBuffer.data(), sizeof(mInteractionsBuffer), 3);
}
#endif

void SimulationHandler::setBounds(float simWidth, float simHeight) {
    mSimWidth  = std::min(std::max(simWidth , MIN_SIM_WIDTH) , MAX_SIM_WIDTH);
    mSimHeight = std::min(std::max(simHeight, MIN_SIM_HEIGHT), MAX_SIM_HEIGHT);
#ifdef ITERATE_ON_COMPUTE_SHADER
    mIterationComputePass1.setUniform(SIMULATION_BOUNDS_UNIFORM, mSimWidth, mSimHeight);
    mIterationComputePass2.setUniform(SIMULATION_BOUNDS_UNIFORM, mSimWidth, mSimHeight);
#endif
}

float SimulationHandler::getWidth() const {
    return mSimWidth;
}

float SimulationHandler::getHeight() const {
    return mSimHeight;
}

void SimulationHandler::setDt(float dt) {
    mDt = std::min(std::max(dt, MIN_DT), MAX_DT);
#ifdef ITERATE_ON_COMPUTE_SHADER
    mIterationComputePass2.setUniform(DT_UNIFORM, mDt);
#endif
}

void SimulationHandler::setDrag(float drag) {
    mDrag = std::min(std::max(drag, MIN_DRAG), MAX_DRAG);
#ifdef ITERATE_ON_COMPUTE_SHADER
    mIterationComputePass2.setUniform(DRAG_FORCE_UNIFORM, mDrag);
#endif
}

void SimulationHandler::setInteractionRange(float interactionRange) {
    mInteractionRange = std::max(interactionRange, MIN_INTERACTION_RANGE);
    mInteractionRange2 = mInteractionRange * mInteractionRange;
#ifdef ITERATE_ON_COMPUTE_SHADER
    mIterationComputePass1.setUniform(INTERACTION_RANGE2_UNIFORM, mInteractionRange2);
#endif
}

void SimulationHandler::setCollisionForce(float collisionForce) {
    mCollisionForce = std::min(std::max(collisionForce, MIN_COLLISION_FORCE), MAX_COLLISION_FORCE);
#ifdef ITERATE_ON_COMPUTE_SHADER
    mIterationComputePass1.setUniform(COLLISION_FORCE_UNIFORM, mCollisionForce);
#endif
}

void SimulationHandler::setAtomDiameter(float atomDiameter) {
    mAtomDiameter = std::max(atomDiameter, MIN_ATOM_DIAMETER);
#ifdef ITERATE_ON_COMPUTE_SHADER
    mIterationComputePass1.setUniform(ATOM_DIAMETER_UNIFORM, mAtomDiameter);
#endif
}

void SimulationHandler::clearAtoms() {
    mAtomCount = 0;
}

void SimulationHandler::initSimulation() {
    Logger::getLogger().logMessage("Initializing Simulation");
    clearAtoms();
    for (size_t at = 0; at < mAtomTypeCount; at++) {
        for (size_t a = 0; a < mAtomTypes[at].quantity; a++) {
            if (mAtomCount >= MAX_ATOMS)
                break;
            mAtomsBuffer[mAtomCount++] = Atom(mAtomTypes[at].id);
        }
    }
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::writeBuffer(mAtomsBufferID, mAtomsBuffer.data(), sizeof(mAtomsBuffer));
#endif
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::readBuffer(mAtomsBufferID, mAtomsBuffer.data(), sizeof(mAtomsBuffer));
#endif
    switch (startCondition) {
        default:
        case StartConditionRandom:            initAtomPositionsRandom();            break;
        case StartConditionEquidistant:       initAtomPositionsEquidistant();       break;
        case StartConditionRandomEquidistant: initAtomPositionsRandomEquidistant(); break;
        case StartConditionRings:             initAtomPositionsRings();             break;
    }
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::writeBuffer(mAtomsBufferID, mAtomsBuffer.data(), sizeof(mAtomsBuffer));
#endif
}

void SimulationHandler::iterateSimulation() {
#ifdef ITERATE_ON_COMPUTE_SHADER
    mIterationComputePass1.run(mAtomCount, mAtomCount, 1);
    mIterationComputePass2.run(mAtomCount, 1, 1);
#else
    for (size_t i = 0; i < mAtomCount; i++) {
        Atom& atomA = mAtomsBuffer[i];
        for (size_t j = 0; j < mAtomCount; j++) {
            if (i == j) continue;
            Atom& atomB = mAtomsBuffer[j];

            float g = mInteractionsBuffer[INTERACTION_INDEX(atomA.atomType, atomB.atomType)];

            float dX = atomA.x - atomB.x;
            float dY = atomA.y - atomB.y;

            float dXAbs = std::abs(dX);
            float dXAlt = mSimWidth - dXAbs;
            dX = (dXAlt < dXAbs) ? dXAlt * (atomA.x < atomB.x ? 1.0f : -1.0f) : dX;

            float dYAbs = std::abs(dY);
            float dYAlt = mSimHeight - dYAbs;
            dY = (dYAlt < dYAbs) ? dYAlt * (atomA.y < atomB.y ? 1.0f : -1.0f) : dY;

            if (dX == 0 && dY == 0)
                continue;

            float d2 = dX * dX + dY * dY;
            if (d2 < mInteractionRange2) {
                float d = std::sqrt(d2);
                float f = g / d;
                f += (d < mAtomDiameter) ? (mAtomDiameter - d) * mCollisionForce / mAtomDiameter : 0.0f;
                atomA.fx += f * dX;
                atomA.fy += f * dY;
            }
        }
    }
    for (Atom& atom : mAtomsBuffer) {
        atom.vx = (atom.vx + atom.fx * mDt) * mDrag;
        atom.vy = (atom.vy + atom.fy * mDt) * mDrag;
        atom.fx = 0.0f;
        atom.fy = 0.0f;
        atom.x += atom.vx * mDt;
        atom.y += atom.vy * mDt;

        atom.x += (atom.x < 0) ? mSimWidth :
            (atom.x >= mSimWidth) ? -mSimWidth : 0.0f;
        atom.y += (atom.y < 0) ? mSimHeight :
            (atom.y >= mSimHeight) ? -mSimHeight : 0.0f;
    }
#endif
}

atom_type_id SimulationHandler::newAtomType() {
    if (mAtomTypeCount >= MAX_ATOM_TYPES)
        return INT_MAX;

    int index = mAtomTypeCount++;
    atom_type_id id = (mAtomTypes[index] = AtomType(index)).id;
    mAtomTypesBuffer[index] = AtomTypeRaw(mAtomTypes[index]);

    for (atom_type_id id2 : getAtomTypeIds()) {
        if (mInteractionCount >= MAX_INTERACTIONS)
            break;
        mInteractionsBuffer[mInteractionCount++] = 0.0f;
        if (id != id2)
            mInteractionsBuffer[mInteractionCount++] = 0.0f;
    }

#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::writeBuffer(mAtomTypesBufferID, mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer));
    BaseShader::writeBuffer(mInteractionsBufferID, mInteractionsBuffer.data(), sizeof(mInteractionsBuffer));
#endif
    return id;
}

void SimulationHandler::removeAtomType(atom_type_id atomTypeId) {
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::readBuffer(mAtomsBufferID, mAtomsBuffer.data(), sizeof(mAtomsBuffer));
#endif

    mAtomCount = std::remove_if(mAtomsBuffer.begin(), mAtomsBuffer.begin() + mAtomCount,
        [atomTypeId](Atom& atom) {
            return atom.atomType == atomTypeId;
        }) - mAtomsBuffer.begin();

    std::array<unsigned int, MAX_ATOM_TYPES> newIndices{};
    int counter = 0;
    for (size_t at = 0; at < mAtomTypeCount; at++)
        if (mAtomTypes[at].id != atomTypeId)
            newIndices[at] = counter++;
    for (size_t a = 0; a < mAtomCount; a++)
        mAtomsBuffer[a].atomType = newIndices[mAtomsBuffer[a].atomType];
    bool toRemove[MAX_INTERACTIONS]{false };
    for (unsigned int i = atomTypeId * atomTypeId; i < (atomTypeId + 1) * (atomTypeId + 1); i++)
        toRemove[i] = true;
    for (unsigned int n = atomTypeId + 1; n < mAtomTypeCount; n++) {
        toRemove[n * n + 2 * atomTypeId + 1] = true;
        toRemove[n * n + 2 * atomTypeId + 2] = true;
    }
    unsigned int index = 0;
    mInteractionCount = std::remove_if(mInteractionsBuffer.begin(), mInteractionsBuffer.begin() + mInteractionCount,
        [&index, toRemove](float& in) {
            return toRemove[index++];
        }) - mInteractionsBuffer.begin();

    mAtomTypeCount = std::remove_if(mAtomTypes.begin(), mAtomTypes.begin() + mAtomTypeCount,
        [atomTypeId](AtomType& atomType) {
            return atomType.id == atomTypeId;
        }) - mAtomTypes.begin();
    for (unsigned int i = 0; i < mAtomTypeCount; i++) {
        mAtomTypes[i].id = i;
        mAtomTypesBuffer[i] = AtomTypeRaw(mAtomTypes[i]);
    }
    mInteractionCount = mAtomTypeCount * mAtomTypeCount;

#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::writeBuffer(mInteractionsBufferID, mInteractionsBuffer.data(), sizeof(mInteractionsBuffer));
    BaseShader::writeBuffer(mAtomsBufferID, mAtomsBuffer.data(), sizeof(mAtomsBuffer));
    BaseShader::writeBuffer(mAtomTypesBufferID, mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer));
#endif
}

void SimulationHandler::clearAtomTypes() {
    clearAtoms();
    mAtomTypeCount = 0;
    mInteractionCount = 0;
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::writeBuffer(mAtomTypesBufferID, mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer));
    BaseShader::writeBuffer(mInteractionsBufferID, mInteractionsBuffer.data(), sizeof(mInteractionsBuffer));
#endif
}

std::vector<atom_type_id> SimulationHandler::getAtomTypeIds() const {
    std::vector<atom_type_id> ids(mAtomTypeCount);
    for (size_t i = 0; i < mAtomTypeCount; i++) {
        ids[i] = mAtomTypes[i].id;
    }
    return ids;
}

void SimulationHandler::setAtomTypeColor(atom_type_id atomTypeId, glm::vec3 color) {
    if (atomTypeId < mAtomTypeCount) {
        mAtomTypes[atomTypeId].r = color.r;
        mAtomTypes[atomTypeId].g = color.g;
        mAtomTypes[atomTypeId].b = color.b;
        mAtomTypesBuffer[atomTypeId] = AtomTypeRaw(mAtomTypes[atomTypeId]);
    }
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::writeBuffer(mAtomTypesBufferID, mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer));
#endif
}

void SimulationHandler::setAtomTypeColorR(atom_type_id atomTypeId, float r) {
    if (atomTypeId < mAtomTypeCount) {
        mAtomTypes[atomTypeId].r = r;
        mAtomTypesBuffer[atomTypeId] = AtomTypeRaw(mAtomTypes[atomTypeId]);
    }
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::writeBuffer(mAtomTypesBufferID, mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer));
#endif
}

void SimulationHandler::setAtomTypeColorG(atom_type_id atomTypeId, float g) {
    if (atomTypeId < mAtomTypeCount) {
        mAtomTypes[atomTypeId].g = g;
        mAtomTypesBuffer[atomTypeId] = AtomTypeRaw(mAtomTypes[atomTypeId]);
    }
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::writeBuffer(mAtomTypesBufferID, mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer));
#endif
}

void SimulationHandler::setAtomTypeColorB(atom_type_id atomTypeId, float b) {
    if (atomTypeId < mAtomTypeCount) {
        mAtomTypes[atomTypeId].b = b;
        mAtomTypesBuffer[atomTypeId] = AtomTypeRaw(mAtomTypes[atomTypeId]);
    }
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::writeBuffer(mAtomTypesBufferID, mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer));
#endif
}

glm::vec3 SimulationHandler::getAtomTypeColor(atom_type_id atomTypeId) const {
    return atomTypeId >= mAtomTypeCount ? glm::vec3{ 0.0f, 0.0f, 0.0f } : glm::vec3{ mAtomTypes[atomTypeId].r, mAtomTypes[atomTypeId].g, mAtomTypes[atomTypeId].b};
}

void SimulationHandler::setAtomTypeQuantity(atom_type_id atomTypeId, unsigned int quantity) {
    if (atomTypeId < mAtomTypeCount) mAtomTypes[atomTypeId].quantity = quantity;
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::writeBuffer(mAtomTypesBufferID, mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer));
#endif
}

unsigned int SimulationHandler::getAtomTypeQuantity(atom_type_id atomTypeId) const {
    return atomTypeId >= mAtomTypeCount ? 0u : mAtomTypes[atomTypeId].quantity;
}

void SimulationHandler::setAtomTypeFriendlyName(atom_type_id atomTypeId, const std::string& friendlyName) {
    if (atomTypeId < mAtomTypeCount) mAtomTypes[atomTypeId].friendlyName = friendlyName;
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::writeBuffer(mAtomTypesBufferID, mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer));
#endif
}

std::string SimulationHandler::getAtomTypeFriendlyName(atom_type_id atomTypeId) const {
    return atomTypeId >= mAtomTypeCount ? std::string() : mAtomTypes[atomTypeId].friendlyName;
}

float SimulationHandler::getInteraction(atom_type_id aId, atom_type_id bId) const {
    return mInteractionsBuffer[INTERACTION_INDEX(aId, bId)];
}

void SimulationHandler::setInteraction(atom_type_id aId, atom_type_id bId, float value) {
    mInteractionsBuffer[INTERACTION_INDEX(aId, bId)] = value;
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::writeBuffer(mInteractionsBufferID, mInteractionsBuffer.data(), sizeof(mInteractionsBuffer));
#endif
}

void SimulationHandler::shuffleAtomInteractions() {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> range(-1.0f, 1.0f);

    for (size_t i = 0; i < mInteractionCount; i++)
        mInteractionsBuffer[i] = range(mt);
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::writeBuffer(mInteractionsBufferID, mInteractionsBuffer.data(), sizeof(mInteractionsBuffer));
#endif
}

void SimulationHandler::zeroAtomInteractions() {
    for (size_t i = 0; i < mInteractionCount; i++)
        mInteractionsBuffer[i] = 0.0f;
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::writeBuffer(mInteractionsBufferID, mInteractionsBuffer.data(), sizeof(mInteractionsBuffer));
#endif
}

size_t SimulationHandler::getAtomCount() const {
    size_t count = 0;
    for (size_t i = 0; i < mAtomTypeCount; i++)
        count += mAtomTypes[i].quantity;
    return count;
}

size_t SimulationHandler::getActualAtomCount() const {
    return mAtomCount;
}

size_t SimulationHandler::getAtomTypeCount() const {
    return mAtomTypeCount;
}

const std::array<Atom, MAX_ATOMS>& SimulationHandler::getAtoms() {
    return mAtomsBuffer;
}

void SimulationHandler::initAtomPositionsRandom() {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> rangeX(0, mSimWidth);
    std::uniform_real_distribution<float> rangeY(0, mSimHeight);

    for (size_t i = 0; i < mAtomCount; i++) {
        mAtomsBuffer[i].x = rangeX(mt);
        mAtomsBuffer[i].y = rangeY(mt);
    }
}

void SimulationHandler::initAtomPositionsEquidistant() {
    size_t rootCount = std::ceil(std::sqrt(mAtomCount));
    size_t delta = rootCount * rootCount - mAtomCount;
    size_t halfD = delta / 2;
    size_t d1 = halfD / 2;
    size_t d2 = halfD - d1;
    size_t d3 = (delta - halfD) / 2;
    size_t d4 = (delta - halfD) - d3;
    size_t index = 0;
    for (size_t i = 0; i < rootCount; i++) {
        for (size_t j = 0; j < rootCount; j++) {
            if (((i == 0) && (j < d1 || j >= rootCount - d2)) || ((i == rootCount - 1) && (j < d3 || j >= rootCount - d4))) continue;
            mAtomsBuffer[index].x = (j + 1) * mSimWidth / (rootCount + 1);
            mAtomsBuffer[index++].y = (i + 1) * mSimHeight / (rootCount + 1);
        }
    }
}

void SimulationHandler::initAtomPositionsRandomEquidistant() {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> range(0, mAtomCount - 1);

    size_t rootCount = std::ceil(std::sqrt(mAtomCount));
    size_t delta = rootCount * rootCount - mAtomCount;
    size_t halfD = delta / 2;
    size_t d1 = halfD / 2;
    size_t d2 = halfD - d1;
    size_t d3 = (delta - halfD) / 2;
    size_t d4 = (delta - halfD) - d3;

    std::vector<size_t> randSequence(mAtomCount);
    for (size_t i = 0; i < mAtomCount; i++)
        randSequence[i] = i;
    for (size_t i = 0; i < mAtomCount; i++) {
        size_t swap = range(mt);
        size_t temp = randSequence[i];
        randSequence[i] = randSequence[swap];
        randSequence[swap] = temp;
    }
    size_t index = 0;
    for (size_t i = 0; i < rootCount; i++) {
        for (size_t j = 0; j < rootCount; j++) {
            if (((i == 0) && (j < d1 || j >= rootCount - d2)) || ((i == rootCount - 1) && (j < d3 || j >= rootCount - d4))) continue;
            mAtomsBuffer[randSequence[index  ]].x = (j + 1) * mSimWidth / (rootCount + 1);
            mAtomsBuffer[randSequence[index++]].y = (i + 1) * mSimHeight / (rootCount + 1);
        }
    }
}

void SimulationHandler::initAtomPositionsRings() {
    float ringDist = std::min(mSimWidth, mSimHeight) / (2.0f * (mAtomTypeCount + 1));
    size_t index = 0;
    for (size_t at = 0; at < mAtomTypeCount; at++) {
        AtomType& atomType = mAtomTypes[at];
        for (size_t i = 0; i < atomType.quantity; i++) {
            float theta = i * (2.0f * 3.141592653589f) / atomType.quantity;
            mAtomsBuffer[index  ].x = std::cos(theta) * ringDist * (at + 1) + mSimWidth / 2;
            mAtomsBuffer[index++].y = std::sin(theta) * ringDist * (at + 1) + mSimHeight / 2;
        }
    }
}
