#include "SimulationHandler.h"

#include <random>
#include <vector>

SimulationHandler::SimulationHandler() :
mSimWidth(0), mSimHeight(0), mDt(1.0f), mDrag(0.5f), mLSRules() {

}

SimulationHandler::~SimulationHandler() {
    clearAtoms();
}

void SimulationHandler::setBounds(float simWidth, float simHeight) {
    mSimWidth = simWidth;
    mSimHeight = simHeight;
}

float SimulationHandler::getWidth() const {
    return mSimWidth;
}

float SimulationHandler::getHeight() const {
    return mSimHeight;
}

void SimulationHandler::setDt(float dt) {
    mDt = dt;
}

float SimulationHandler::getDt() {
    return mDt;
}

void SimulationHandler::setDrag(float drag) {
    mDrag = drag;
}

float SimulationHandler::getDrag() {
    return mDrag;
}

void SimulationHandler::clearAtoms() {
    for (AtomType& atomType : mLSRules.getAtomTypes()) {
        atomType.clearAtoms();
    }
}

void SimulationHandler::clearAtomTypes() {
    clearAtoms();
    mLSRules.clearAtomTypes();
}

void SimulationHandler::initSimulation() {
    clearAtoms();

    for (AtomType& atomType : mLSRules.getAtomTypes()) {
        unsigned int quantity = atomType.getQuantity();
        for (int i = 0; i < quantity; i++) {
            atomType.newAtom();
        }
    }
    shuffleAtomPositions();
}

void SimulationHandler::iterateSimulation() {
    float atomRadius = mLSRules.getAtomRadius();
    float atomDiameter = atomRadius * 2;
    float atomRadius2 = atomRadius * atomRadius;
    std::vector<AtomType>& atomTypes = mLSRules.getAtomTypes();
    for (AtomType& atA : atomTypes) {
        for (AtomType& atB : atomTypes) {
            float g = mLSRules.getInteraction(atA.getId(), atB.getId());
            for (Atom& atomA : atA.getAtoms()) {
                for (Atom& atomB : atB.getAtoms()) {
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
                    if (d2 < 6400) {
                        float d = sqrt(d2);
                        float f = g / d;
                        f += (d < atomDiameter) ? (atomDiameter - d) * 1.0f / atomDiameter : 0.0f;
                        atomA.mFX += f * dX;
                        atomA.mFY += f * dY;
                    }
                }
            }
        }
    }
    for (AtomType& atomType : atomTypes) {
        for (Atom& atom : atomType.getAtoms()) {
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
    }
}

void SimulationHandler::removeAtomType(unsigned int atomTypeId) {
    mLSRules.removeAtomType(atomTypeId);
}

void SimulationHandler::shuffleAtomPositions() {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> rangeX(0, mSimWidth);
    std::uniform_real_distribution<float> rangeY(0, mSimWidth);

    for (AtomType& atomType : mLSRules.getAtomTypes()) {
        for (Atom& atom : atomType.getAtoms()) {
            atom.mX = rangeX(mt);
            atom.mY = rangeY(mt);
            atom.mVX = 0.0f;
            atom.mVY = 0.0f;
            atom.mFX = 0.0f;
            atom.mFY = 0.0f;
        }
    }
}

void SimulationHandler::shuffleAtomInteractions() {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> range(-1.0f, 1.0f);

    std::vector<AtomType>& atomTypes = mLSRules.getAtomTypes();

    for (AtomType& atomTypeA : atomTypes) {
        for (AtomType& atomTypeB : atomTypes) {
            mLSRules.setInteraction(atomTypeA.getId(), atomTypeB.getId(), range(mt));
        }
    }
}

SimulationRules& SimulationHandler::getLSRules() {
    return mLSRules;
}
