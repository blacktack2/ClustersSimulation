#include "SimulationHandler.h"

#include <algorithm>
#include <random>
#ifdef ITERATE_ON_COMPUTE_SHADER
#include <climits>
#include <iostream>
#endif

SimulationHandler::SimulationHandler() :
mSimWidth(0), mSimHeight(0), mDt(1.0f), mDrag(0.5f),
mInteractionRange(80), mInteractionRange2(6400), mCollisionForce(1.0f)
#ifdef ITERATE_ON_COMPUTE_SHADER
, mIterationComputePass1("IterationPass1.comp"), mIterationComputePass2("IterationPass2.comp"),
mAtomTypesBufferID(), mAtomsBufferID(), mInteractionsBufferID()
#endif
, mAtomCount(0), mAtomTypeCount(0), mInteractionCount(0),
mAtomTypes(), mAtomTypesBuffer(), mAtomsBuffer(), mInteractionsBuffer()
{
}

SimulationHandler::~SimulationHandler() {
    clearAtoms();
}

#ifdef ITERATE_ON_COMPUTE_SHADER
void SimulationHandler::initComputeShaders() {
    mIterationComputePass1.init();
    if (!mIterationComputePass1.isValid()) {
        fprintf(stderr, "Failed to create Compute Shader\n");
        return;
    }
    mIterationComputePass2.init();
    if (!mIterationComputePass2.isValid()) {
        fprintf(stderr, "Failed to create Compute Shader\n");
        return;
    }
    mAtomTypesBufferID    = BaseShader::createBuffer(mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer), 1);
    mAtomsBufferID        = BaseShader::createBuffer(mAtomsBuffer.data(), sizeof(mAtomsBuffer), 2);
    mInteractionsBufferID = BaseShader::createBuffer(mInteractionsBuffer.data(), sizeof(mInteractionsBuffer), 3);
}
#endif

void SimulationHandler::setBounds(float simWidth, float simHeight) {
    mSimWidth = simWidth;
    mSimHeight = simHeight;
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::setUniform(SIMULATION_BOUNDS_UNIFORM, mSimWidth, mSimHeight);
#endif
}

float SimulationHandler::getWidth() const {
    return mSimWidth;
}

float SimulationHandler::getHeight() const {
    return mSimHeight;
}

void SimulationHandler::setDt(float dt) {
    mDt = dt;
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::setUniform(DT_UNIFORM, mDt);
#endif
}

void SimulationHandler::setDrag(float drag) {
    mDrag = drag;
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::setUniform(DRAG_FORCE_UNIFORM, mDrag);
#endif
}

void SimulationHandler::setInteractionRange(float interactionRange) {
    mInteractionRange = interactionRange;
    mInteractionRange2 = mInteractionRange * mInteractionRange;
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::setUniform(INTERACTION_RANGE2_UNIFORM, mInteractionRange2);
#endif
}

void SimulationHandler::setCollisionForce(float collisionForce) {
    mCollisionForce = collisionForce;
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::setUniform(COLLISION_FORCE_UNIFORM, mCollisionForce);
#endif
}

void SimulationHandler::clearAtoms() {
    mAtomCount = 0;
}

void SimulationHandler::initSimulation() {
    clearAtoms();

    for (int at = 0; at < mAtomTypeCount; at++) {
        for (int a = 0; a < mAtomTypes[at].quantity; a++) {
            if (mAtomCount >= MAX_ATOMS)
                break;
            mAtomsBuffer[mAtomCount++] = Atom(mAtomTypes[at].id);
        }
    }
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::writeBuffer(mAtomsBufferID, mAtomsBuffer.data(), sizeof(mAtomsBuffer));
#endif
    shuffleAtomPositions();
}

void SimulationHandler::iterateSimulation() {
#ifdef ITERATE_ON_COMPUTE_SHADER
    mIterationComputePass1.run(mAtomCount, mAtomCount, 1);
    mIterationComputePass2.run(mAtomCount, 1, 1);
#else
    float atomRadius = 3.0f;
    float atomDiameter = atomRadius * 2;
    for (int i = 0; i < mAtomCount; i++) {
        Atom& atomA = mAtomsBuffer[i];
        for (int j = 0; j < mAtomCount; j++) {
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
                f += (d < atomDiameter) ? (atomDiameter - d) * mCollisionForce / atomDiameter : 0.0f;
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

unsigned int SimulationHandler::newAtomType() {
    if (mAtomTypeCount >= MAX_ATOM_TYPES)
        return INT_MAX;

    int index = mAtomTypeCount++;
    unsigned int id = (mAtomTypes[index] = AtomType(index)).id;
    mAtomTypesBuffer[index] = AtomTypeRaw(mAtomTypes[index]);

    for (unsigned int id2 : getAtomTypeIds()) {
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

void SimulationHandler::removeAtomType(unsigned int atomTypeId) {
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::readBuffer(mAtomsBufferID, mAtomsBuffer.data(), sizeof(mAtomsBuffer));
#endif

    mAtomCount = std::remove_if(mAtomsBuffer.begin(), mAtomsBuffer.begin() + mAtomCount,
        [atomTypeId](Atom& atom) {
            return atom.atomType == atomTypeId;
        }) - mAtomsBuffer.begin();

    std::array<unsigned int, MAX_ATOM_TYPES> newIndices{};
    int counter = 0;
    for (int at = 0; at < mAtomTypeCount; at++)
        if (mAtomTypes[at].id != atomTypeId)
            newIndices[at] = counter++;
    for (int a = 0; a < mAtomCount; a++)
        mAtomsBuffer[a].atomType = newIndices[mAtomsBuffer[a].atomType];
    bool toRemove[MAX_INTERACTIONS]{false };
    for (unsigned int i = atomTypeId * atomTypeId; i < (atomTypeId + 1) * (atomTypeId + 1); i++)
        toRemove[i] = true;
    for (unsigned int n = atomTypeId + 1; n < mAtomTypeCount; n++) {
        toRemove[n * n + 2 * atomTypeId + 1] = true;
        toRemove[n * n + 2 * atomTypeId + 2] = true;
    }
    int index = 0;
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

std::vector<unsigned int> SimulationHandler::getAtomTypeIds() const {
    std::vector<unsigned int> ids(mAtomTypeCount);
    for (int i = 0; i < mAtomTypeCount; i++) {
        ids[i] = mAtomTypes[i].id;
    }
    return ids;
}

void SimulationHandler::setAtomTypeColor(unsigned int atomTypeId, Color color) {
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

void SimulationHandler::setAtomTypeColorR(unsigned int atomTypeId, float r) {
    if (atomTypeId < mAtomTypeCount) {
        mAtomTypes[atomTypeId].r = r;
        mAtomTypesBuffer[atomTypeId] = AtomTypeRaw(mAtomTypes[atomTypeId]);
    }
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::writeBuffer(mAtomTypesBufferID, mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer));
#endif
}

void SimulationHandler::setAtomTypeColorG(unsigned int atomTypeId, float g) {
    if (atomTypeId < mAtomTypeCount) {
        mAtomTypes[atomTypeId].g = g;
        mAtomTypesBuffer[atomTypeId] = AtomTypeRaw(mAtomTypes[atomTypeId]);
    }
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::writeBuffer(mAtomTypesBufferID, mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer));
#endif
}

void SimulationHandler::setAtomTypeColorB(unsigned int atomTypeId, float b) {
    if (atomTypeId < mAtomTypeCount) {
        mAtomTypes[atomTypeId].b = b;
        mAtomTypesBuffer[atomTypeId] = AtomTypeRaw(mAtomTypes[atomTypeId]);
    }
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::writeBuffer(mAtomTypesBufferID, mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer));
#endif
}

Color SimulationHandler::getAtomTypeColor(unsigned int atomTypeId) const {
    return atomTypeId >= mAtomTypeCount ? Color{ 0.0f, 0.0f, 0.0f } : Color{ mAtomTypes[atomTypeId].r, mAtomTypes[atomTypeId].g, mAtomTypes[atomTypeId].b};
}

void SimulationHandler::setAtomTypeQuantity(unsigned int atomTypeId, unsigned int quantity) {
    if (atomTypeId < mAtomTypeCount) mAtomTypes[atomTypeId].quantity = quantity;
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::writeBuffer(mAtomTypesBufferID, mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer));
#endif
}

unsigned int SimulationHandler::getAtomTypeQuantity(unsigned int atomTypeId) const {
    return atomTypeId >= mAtomTypeCount ? 0u : mAtomTypes[atomTypeId].quantity;
}

void SimulationHandler::setAtomTypeFriendlyName(unsigned int atomTypeId, const std::string& friendlyName) {
    if (atomTypeId < mAtomTypeCount) mAtomTypes[atomTypeId].friendlyName = friendlyName;
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::writeBuffer(mAtomTypesBufferID, mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer));
#endif
}

std::string SimulationHandler::getAtomTypeFriendlyName(unsigned int atomTypeId) const {
    return atomTypeId >= mAtomTypeCount ? std::string() : mAtomTypes[atomTypeId].friendlyName;
}

float SimulationHandler::getInteraction(unsigned int aId, unsigned int bId) const {
    return mInteractionsBuffer[INTERACTION_INDEX(aId, bId)];
}

void SimulationHandler::setInteraction(unsigned int aId, unsigned int bId, float value) {
    mInteractionsBuffer[INTERACTION_INDEX(aId, bId)] = value;
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::writeBuffer(mInteractionsBufferID, mInteractionsBuffer.data(), sizeof(mInteractionsBuffer));
#endif
}

void SimulationHandler::shuffleAtomPositions() {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> rangeX(0, mSimWidth);
    std::uniform_real_distribution<float> rangeY(0, mSimWidth);

#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::readBuffer(mAtomsBufferID, mAtomsBuffer.data(), sizeof(mAtomsBuffer));
#endif

    for (int i = 0; i < mAtomCount; i++) {
        mAtomsBuffer[i].x = rangeX(mt);
        mAtomsBuffer[i].y = rangeY(mt);
    }

#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::writeBuffer(mAtomsBufferID, mAtomsBuffer.data(), sizeof(mAtomsBuffer));
#endif
}

void SimulationHandler::shuffleAtomInteractions() {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> range(-1.0f, 1.0f);

    for (int i = 0; i < mInteractionCount; i++)
        mInteractionsBuffer[i] = range(mt);
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::writeBuffer(mInteractionsBufferID, mInteractionsBuffer.data(), sizeof(mInteractionsBuffer));
#endif
}

void SimulationHandler::zeroAtomInteractions() {
    for (int i = 0; i < mInteractionCount; i++)
        mInteractionsBuffer[i] = 0.0f;
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::writeBuffer(mInteractionsBufferID, mInteractionsBuffer.data(), sizeof(mInteractionsBuffer));
#endif
}

unsigned int SimulationHandler::getAtomCount() const {
    unsigned int count = 0;
    for (int i = 0; i < mAtomTypeCount; i++)
        count += mAtomTypes[i].quantity;
    return count;
}

unsigned int SimulationHandler::getActualAtomCount() const {
    return mAtomCount;
}

unsigned int SimulationHandler::getAtomTypeCount() const {
    return mAtomTypeCount;
}

const std::array<Atom, MAX_ATOMS>& SimulationHandler::getAtoms() {
    return mAtomsBuffer;
}
