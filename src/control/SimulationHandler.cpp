#include "SimulationHandler.h"

#include <algorithm>
#include <climits>
#include <random>
#ifdef ITERATE_ON_COMPUTE_SHADER
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#endif

SimulationHandler::SimulationHandler() :
mSimWidth(0), mSimHeight(0), mDt(1.0f), mDrag(0.5f),
mInteractionRange(80), mInteractionRange2(6400), mCollisionForce(1.0f)
#ifdef ITERATE_ON_COMPUTE_SHADER
, mIterationComputePass1("IterationPass1.comp"), mIterationComputePass2("IterationPass2.comp"),
mAtomCount(0), mAtomTypeCount(0), mInteractionCount(0),
mAtomTypesBufferID(), mAtomsBufferID(), mInteractionsBufferID(),
mAtomTypes(), mAtomTypesBuffer(), mAtomsBuffer(), mInteractionsBuffer()
#else
, mAtoms(), mLSRules()
#endif
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

float SimulationHandler::getDt() {
    return mDt;
}

void SimulationHandler::setDrag(float drag) {
    mDrag = drag;
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::setUniform(DRAG_FORCE_UNIFORM, mDrag);
#endif
}

float SimulationHandler::getDrag() {
    return mDrag;
}

void SimulationHandler::setInteractionRange(float interactionRange) {
    mInteractionRange = interactionRange;
    mInteractionRange2 = mInteractionRange * mInteractionRange;
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::setUniform(INTERACTION_RANGE2_UNIFORM, mInteractionRange2);
#endif
}

float SimulationHandler::getInteractionRange() {
    return mInteractionRange;
}

void SimulationHandler::setCollisionForce(float collisionForce) {
    mCollisionForce = collisionForce;
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::setUniform(COLLISION_FORCE_UNIFORM, mCollisionForce);
#endif
}

float SimulationHandler::getCollisionForce() {
    return mCollisionForce;
}

void SimulationHandler::clearAtoms() {
#ifdef ITERATE_ON_COMPUTE_SHADER
    mAtomCount = 0;
#else
    mAtoms.clear();
#endif
}

void SimulationHandler::initSimulation() {
    clearAtoms();

#ifdef ITERATE_ON_COMPUTE_SHADER
    for (int at = 0; at < mAtomTypeCount; at++) {
        for (int a = 0; a < mAtomTypes[at].quantity; a++) {
            if (mAtomCount >= ATOMS_BUFFER_SIZE)
                break;
            mAtomsBuffer[mAtomCount++] = Atom(mAtomTypes[at].id);
        }
    }
    BaseShader::writeBuffer(mAtomsBufferID, mAtomsBuffer.data(), sizeof(mAtomsBuffer));
#else
    for (std::unique_ptr<AtomType>& atomType : mLSRules.getAtomTypes()) {
        unsigned int quantity = atomType->getQuantity();
        for (int i = 0; i < quantity; i++) {
            mAtoms.emplace_back(atomType.get());
        }
    }
#endif
    shuffleAtomPositions();
}

void SimulationHandler::iterateSimulation() {
#ifdef ITERATE_ON_COMPUTE_SHADER
    mIterationComputePass1.run(mAtomCount, mAtomCount, 1);
    mIterationComputePass2.run(mAtomCount, 1, 1);
#else
    float atomRadius = mLSRules.getAtomRadius();
    float atomDiameter = atomRadius * 2;
    float atomRadius2 = atomRadius * atomRadius;
    for (Atom& atomA : mAtoms) {
        for (Atom& atomB : mAtoms) {
            float g = mLSRules.getInteraction(atomA.mAtomType->getId(), atomB.mAtomType->getId());
            if (&atomA == &atomB) {
                continue;
            }

            float dX = atomA.mX - atomB.mX;
            float dY = atomA.mY - atomB.mY;

            float dXAbs = abs(dX);
            float dXAlt = mSimWidth - dXAbs;
            dX = (dXAlt < dXAbs) ? dXAlt * (atomA.mX < atomB.mX ? 1.0f : -1.0f) : dX;

            float dYAbs = abs(dY);
            float dYAlt = mSimHeight - dYAbs;
            dY = (dYAlt < dYAbs) ? dYAlt * (atomA.mY < atomB.mY ? 1.0f : -1.0f) : dY;

            if (dX == 0 && dY == 0) {
                continue;
            }

            float d2 = dX * dX + dY * dY;
            if (d2 < mInteractionRange2) {
                float d = sqrt(d2);
                float f = g / d;
                f += (d < atomDiameter) ? (atomDiameter - d) * mCollisionForce / atomDiameter : 0.0f;
                atomA.mFX += f * dX;
                atomA.mFY += f * dY;
            }
        }
    }
    for (Atom& atom : mAtoms) {
        atom.mVX = (atom.mVX + atom.mFX * mDt) * mDrag;
        atom.mVY = (atom.mVY + atom.mFY * mDt) * mDrag;
        atom.mFX = 0.0f;
        atom.mFY = 0.0f;
        atom.mX += atom.mVX * mDt;
        atom.mY += atom.mVY * mDt;

        atom.mX += (atom.mX < 0) ? mSimWidth :
            (atom.mX >= mSimWidth) ? -mSimWidth : 0.0f;
        atom.mY += (atom.mY < 0) ? mSimHeight :
            (atom.mY >= mSimHeight) ? -mSimHeight : 0.0f;
    }
#endif
}

unsigned int SimulationHandler::newAtomType() {
#ifdef ITERATE_ON_COMPUTE_SHADER
    if (mAtomTypeCount >= ATOM_TYPES_BUFFER_SIZE)
        return INT_MAX;
    int index = mAtomTypeCount++;
    unsigned int id = (mAtomTypes[index] = AtomType(index)).id;
    mAtomTypesBuffer[index] = AtomTypeRaw(mAtomTypes[index]);
    BaseShader::writeBuffer(mAtomTypesBufferID, mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer));
    for (unsigned int id2 : getAtomTypeIds()) {
        if (mInteractionCount >= INTERACTIONS_BUFFER_SIZE)
            break;
        mInteractionsBuffer[mInteractionCount++] = 0.0f;
        if (id != id2)
            mInteractionsBuffer[mInteractionCount++] = 0.0f;
    }
    BaseShader::writeBuffer(mInteractionsBufferID, mInteractionsBuffer.data(), sizeof(mInteractionsBuffer));
    return id;
#else
    return mLSRules.newAtomType()->getId();
#endif
}

void SimulationHandler::removeAtomType(unsigned int atomTypeId) {
#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::readBuffer(mAtomsBufferID, mAtomsBuffer.data(), sizeof(mAtomsBuffer));
    auto dummy1 = std::remove_if(mAtomsBuffer.begin(), mAtomsBuffer.begin() + mAtomCount,
        [atomTypeId, this](Atom& atom) {
            if (atom.atomType == atomTypeId) {
                mAtomCount--;
                return true;
            }
            return false;
        });

    std::array<unsigned int, ATOM_TYPES_BUFFER_SIZE> newIndices{};
    int counter = 0;
    for (int at = 0; at < mAtomTypeCount; at++)
        if (mAtomTypes[at].id != atomTypeId)
            newIndices[at] = counter++;
    for (int a = 0; a < mAtomCount; a++)
        mAtomsBuffer[a].atomType = newIndices[mAtomsBuffer[a].atomType];
    bool toRemove[INTERACTIONS_BUFFER_SIZE]{ false };
    for (int i = atomTypeId * atomTypeId; i < (atomTypeId + 1) * (atomTypeId + 1); i++)
        toRemove[i] = true;
    for (int n = atomTypeId + 1; n < mAtomTypeCount; n++) {
        toRemove[n * n + 2 * atomTypeId + 1] = true;
        toRemove[n * n + 2 * atomTypeId + 2] = true;
    }
    int index = 0;
    auto dummy2 = std::remove_if(mInteractionsBuffer.begin(), mInteractionsBuffer.begin() + mInteractionCount,
        [atomTypeId, this, &index, toRemove](float& in) {
            return toRemove[index++];
        });

    auto dummy3 = std::remove_if(mAtomTypes.begin(), mAtomTypes.begin() + mAtomTypeCount,
        [atomTypeId, this](AtomType& atomType) {
            if (atomType.id == atomTypeId) {
                mAtomTypeCount--;
                return true;
            }
            return false;
        });
    for (unsigned int i = 0; i < mAtomTypeCount; i++) {
        mAtomTypes[i].id = i;
        mAtomTypesBuffer[i] = AtomTypeRaw(mAtomTypes[i]);
    }
    mInteractionCount = mAtomTypeCount * mAtomTypeCount;
    BaseShader::writeBuffer(mInteractionsBufferID, mInteractionsBuffer.data(), sizeof(mInteractionsBuffer));
    BaseShader::writeBuffer(mAtomsBufferID, mAtomsBuffer.data(), sizeof(mAtomsBuffer));
    BaseShader::writeBuffer(mAtomTypesBufferID, mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer));
#else
    mAtoms.erase(
        std::remove_if(
            mAtoms.begin(), mAtoms.end(),
            [atomTypeId](Atom& atom) {
                return atom.mAtomType->getId() == atomTypeId;
            }), mAtoms.end()
                );
    mLSRules.removeAtomType(atomTypeId);
#endif
}

void SimulationHandler::clearAtomTypes() {
    clearAtoms();
#ifdef ITERATE_ON_COMPUTE_SHADER
    mAtomTypeCount = 0;
    BaseShader::writeBuffer(mAtomTypesBufferID, mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer));
    mInteractionCount = 0;
    BaseShader::writeBuffer(mInteractionsBufferID, mInteractionsBuffer.data(), sizeof(mInteractionsBuffer));
#else
    mLSRules.clearAtomTypes();
#endif
}

std::vector<unsigned int> SimulationHandler::getAtomTypeIds() {
#ifdef ITERATE_ON_COMPUTE_SHADER
    std::vector<unsigned int> ids(mAtomTypeCount);
    for (int i = 0; i < mAtomTypeCount; i++) {
        ids[i] = mAtomTypes[i].id;
    }
#else
    std::vector<std::unique_ptr<AtomType>>& atomTypes = mLSRules.getAtomTypes();
    std::vector<unsigned int> ids(atomTypes.size());
    for (int i = 0; i < atomTypes.size(); i++) {
        ids[i] = atomTypes[i]->getId();
    }
#endif
    return ids;
}

void SimulationHandler::setAtomTypeColor(unsigned int atomTypeId, Color color) {
#ifdef ITERATE_ON_COMPUTE_SHADER
    for (int i = 0; i < mAtomTypeCount; i++) {
        if (mAtomTypes[i].id == atomTypeId) {
            mAtomTypes[i].r = color.r;
            mAtomTypes[i].g = color.g;
            mAtomTypes[i].b = color.b;
            mAtomTypesBuffer[i] = AtomTypeRaw(mAtomTypes[i]);
            break;
        }
    }
    BaseShader::writeBuffer(mAtomTypesBufferID, mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer));
#else
    AtomType* at = mLSRules.getAtomType(atomTypeId);
    if (at != nullptr) {
        at->setColor(color);
    }
#endif
}

void SimulationHandler::setAtomTypeColorR(unsigned int atomTypeId, float r) {
#ifdef ITERATE_ON_COMPUTE_SHADER
    for (int i = 0; i < mAtomTypeCount; i++) {
        if (mAtomTypes[i].id == atomTypeId) {
            mAtomTypes[i].r = r;
            mAtomTypesBuffer[i] = AtomTypeRaw(mAtomTypes[i]);
            break;
        }
    }
    BaseShader::writeBuffer(mAtomTypesBufferID, mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer));
#else
    AtomType* at = mLSRules.getAtomType(atomTypeId);
    if (at != nullptr) {
        at->setColorR(r);
    }
#endif
}

void SimulationHandler::setAtomTypeColorG(unsigned int atomTypeId, float g) {
#ifdef ITERATE_ON_COMPUTE_SHADER
    for (int i = 0; i < mAtomTypeCount; i++) {
        if (mAtomTypes[i].id == atomTypeId) {
            mAtomTypes[i].g = g;
            mAtomTypesBuffer[i] = AtomTypeRaw(mAtomTypes[i]);
            break;
        }
    }
    BaseShader::writeBuffer(mAtomTypesBufferID, mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer));
#else
    AtomType* at = mLSRules.getAtomType(atomTypeId);
    if (at != nullptr) {
        at->setColorG(g);
    }
#endif
}

void SimulationHandler::setAtomTypeColorB(unsigned int atomTypeId, float b) {
#ifdef ITERATE_ON_COMPUTE_SHADER
    for (int i = 0; i < mAtomTypeCount; i++) {
        if (mAtomTypes[i].id == atomTypeId) {
            mAtomTypes[i].b = b;
            mAtomTypesBuffer[i] = AtomTypeRaw(mAtomTypes[i]);
            break;
        }
    }
    BaseShader::writeBuffer(mAtomTypesBufferID, mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer));
#else
    AtomType* at = mLSRules.getAtomType(atomTypeId);
    if (at != nullptr) {
        at->setColorB(b);
    }
#endif
}

Color SimulationHandler::getAtomTypeColor(unsigned int atomTypeId) {
#ifdef ITERATE_ON_COMPUTE_SHADER
    for (AtomType& atomType : mAtomTypes) {
        if (atomType.id == atomTypeId) {
            return { atomType.r, atomType.g, atomType.b };
        }
    }
#else
    AtomType* at = mLSRules.getAtomType(atomTypeId);
    if (at != nullptr) {
        return at->getColor();
    }
#endif
    return { 0.0f, 0.0f, 0.0f };
}

void SimulationHandler::setAtomTypeQuantity(unsigned int atomTypeId, unsigned int quantity) {
#ifdef ITERATE_ON_COMPUTE_SHADER
    for (int at = 0; at < mAtomTypeCount; at++) {
        if (mAtomTypes[at].id == atomTypeId) {
            mAtomTypes[at].quantity = quantity;
        }
    }
    BaseShader::writeBuffer(mAtomTypesBufferID, mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer));
#else
    AtomType* at = mLSRules.getAtomType(atomTypeId);
    if (at != nullptr) {
        at->setQuantity(quantity);
    }
#endif
}

unsigned int SimulationHandler::getAtomTypeQuantity(unsigned int atomTypeId) {
#ifdef ITERATE_ON_COMPUTE_SHADER
    for (int at = 0; at < mAtomTypeCount; at++) {
        if (mAtomTypes[at].id == atomTypeId) {
            return mAtomTypes[at].quantity;
        }
    }
#else
    AtomType* at = mLSRules.getAtomType(atomTypeId);
    if (at != nullptr) {
        return at->getQuantity();
    }
#endif
    return 0;
}

void SimulationHandler::setAtomTypeFriendlyName(unsigned int atomTypeId, std::string friendlyName) {
#ifdef ITERATE_ON_COMPUTE_SHADER
    for (int at = 0; at < mAtomTypeCount; at++) {
        if (mAtomTypes[at].id == atomTypeId) {
            mAtomTypes[at].friendlyName = friendlyName;
        }
    }
    BaseShader::writeBuffer(mAtomTypesBufferID, mAtomTypesBuffer.data(), sizeof(mAtomTypesBuffer));
#else
    AtomType* at = mLSRules.getAtomType(atomTypeId);
    if (at != nullptr) {
        at->setFriendlyName(friendlyName);
    }
#endif
}

std::string SimulationHandler::getAtomTypeFriendlyName(unsigned int atomTypeId) {
#ifdef ITERATE_ON_COMPUTE_SHADER
    for (int at = 0; at < mAtomTypeCount; at++) {
        if (mAtomTypes[at].id == atomTypeId) {
            return mAtomTypes[at].friendlyName;
        }
    }
#else
    AtomType* at = mLSRules.getAtomType(atomTypeId);
    if (at != nullptr) {
        return at->getFriendlyName();
    }
#endif
    return "";
}

float SimulationHandler::getInteraction(unsigned int aId, unsigned int bId) {
#ifdef ITERATE_ON_COMPUTE_SHADER
    return mInteractionsBuffer[INTERACTION_INDEX(aId, bId)];
#else
    return mLSRules.getInteraction(aId, bId);
#endif
}

void SimulationHandler::setInteraction(unsigned int aId, unsigned int bId, float value) {
#ifdef ITERATE_ON_COMPUTE_SHADER
    mInteractionsBuffer[INTERACTION_INDEX(aId, bId)] = value;
    BaseShader::writeBuffer(mInteractionsBufferID, mInteractionsBuffer.data(), sizeof(mInteractionsBuffer));
#else
    mLSRules.setInteraction(aId, bId, value);
#endif
}

void SimulationHandler::shuffleAtomPositions() {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> rangeX(0, mSimWidth);
    std::uniform_real_distribution<float> rangeY(0, mSimWidth);

#ifdef ITERATE_ON_COMPUTE_SHADER
    BaseShader::readBuffer(mAtomsBufferID, mAtomsBuffer.data(), sizeof(mAtomsBuffer));
    for (int i = 0; i < mAtomCount; i++) {
        mAtomsBuffer[i].x = rangeX(mt);
        mAtomsBuffer[i].y = rangeY(mt);
    }
    BaseShader::writeBuffer(mAtomsBufferID, mAtomsBuffer.data(), sizeof(mAtomsBuffer));
#else
    for (Atom& atom : mAtoms) {
        atom.mX = rangeX(mt);
        atom.mY = rangeY(mt);
        atom.mVX = 0.0f;
        atom.mVY = 0.0f;
        atom.mFX = 0.0f;
        atom.mFY = 0.0f;
    }
#endif
}

void SimulationHandler::shuffleAtomInteractions() {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> range(-1.0f, 1.0f);

#ifdef ITERATE_ON_COMPUTE_SHADER
    for (int i = 0; i < mInteractionCount; i++) {
        mInteractionsBuffer[i] = range(mt);
    }
    BaseShader::writeBuffer(mInteractionsBufferID, mInteractionsBuffer.data(), sizeof(mInteractionsBuffer));
#else
    std::vector<std::unique_ptr<AtomType>>& atomTypes = mLSRules.getAtomTypes();

    for (std::unique_ptr<AtomType>& atomTypeA : atomTypes) {
        for (std::unique_ptr<AtomType>& atomTypeB : atomTypes) {
            mLSRules.setInteraction(atomTypeA->getId(), atomTypeB->getId(), range(mt));
        }
    }
#endif
}

void SimulationHandler::zeroAtomInteractions() {
#ifdef ITERATE_ON_COMPUTE_SHADER
    for (int i = 0; i < mInteractionCount; i++) {
        mInteractionsBuffer[i] = 0.0f;
    }
    BaseShader::writeBuffer(mInteractionsBufferID, mInteractionsBuffer.data(), sizeof(mInteractionsBuffer));
#else
    mLSRules.clearInteractions();
#endif
}

unsigned int SimulationHandler::getAtomCount() {
#ifdef ITERATE_ON_COMPUTE_SHADER
    return mAtomCount;
#else
    return mLSRules.getAtomCount();
#endif
}

#ifdef ITERATE_ON_COMPUTE_SHADER
std::array<Atom, ATOMS_BUFFER_SIZE>& SimulationHandler::getAtoms() {
    BaseShader::readBuffer(mAtomsBufferID, mAtomsBuffer.data(), sizeof(mAtomsBuffer));
    return mAtomsBuffer;
#else
std::vector<Atom>& SimulationHandler::getAtoms() {
    return mAtoms;
#endif
}
