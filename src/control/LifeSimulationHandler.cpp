#include "LifeSimulationHandler.h"

#include <random>

LifeSimulationHandler::LifeSimulationHandler() :
mSimWidth(0), mSimHeight(0), mDt(1.0f), mDrag(0.5f), mLSRules(), mAtoms() {
    AtomType* red = mLSRules.newAtomType();
    red->setColor({1.0f, 0.0f, 0.0f});
    red->setFriendlyName("Red");

    AtomType* green = mLSRules.newAtomType();
    green->setColor({0.0f, 1.0f, 0.0f});
    green->setFriendlyName("Green");

    AtomType* blue = mLSRules.newAtomType();
    blue->setColor({0.0f, 0.0f, 1.0f});
    blue->setFriendlyName("Blue");

    AtomType* cyan = mLSRules.newAtomType();
    cyan->setColor({0.0f, 1.0f, 1.0f});
    cyan->setFriendlyName("Cyan");

    AtomType* magenta = mLSRules.newAtomType();
    magenta->setColor({1.0f, 0.0f, 1.0f});
    magenta->setFriendlyName("Magenta");

    AtomType* yellow = mLSRules.newAtomType();
    yellow->setColor({1.0f, 1.0f, 0.0f});
    yellow->setFriendlyName("Yellow");

    shuffleAtomInteractions();
}

LifeSimulationHandler::~LifeSimulationHandler() {
    clearAtoms();
}

void LifeSimulationHandler::setBounds(float simWidth, float simHeight) {
    mSimWidth = simWidth;
    mSimHeight = simHeight;
}

float LifeSimulationHandler::getWidth() const {
    return mSimWidth;
}

float LifeSimulationHandler::getHeight() const {
    return mSimHeight;
}

void LifeSimulationHandler::setDt(float dt) {
    mDt = dt;
}

float LifeSimulationHandler::getDt() {
    return mDt;
}

void LifeSimulationHandler::setDrag(float drag) {
    mDrag = drag;
}

float LifeSimulationHandler::getDrag() {
    return mDrag;
}

void LifeSimulationHandler::clearAtoms() {
    for (Atom* atom : mAtoms) {
        delete atom;
    }
    mAtoms.clear();
}

void LifeSimulationHandler::clearAtomTypes() {
    clearAtoms();
    mLSRules.clearAtomTypes();
}

void LifeSimulationHandler::initSimulation() {
    clearAtoms();

    for (AtomType* atomType : mLSRules.getAtomTypes()) {
        unsigned int quantity = atomType->getQuantity();
        for (int i = 0; i < quantity; i++) {
            Atom* atom = new Atom(atomType);
            mAtoms.push_back(atom);
        }
    }
    shuffleAtomPositions();
}

void LifeSimulationHandler::iterateSimulation() {
    float atomRadius = mLSRules.getAtomRadius();
    float atomDiameter = atomRadius * 2;
    float atomRadius2 = atomRadius * atomRadius;
    for (Atom* atomA : mAtoms) {
        float fX = 0;
        float fY = 0;
        for (Atom* atomB : mAtoms) {
            if (&atomA == &atomB) {
                continue;
            }
            float g = mLSRules.getInteraction(atomA->getAtomType()->getId(), atomB->getAtomType()->getId());
           /* if (g == 0) {
                continue;
            }*/

            float dX = atomA->mX - atomB->mX;
            float dY = atomA->mY - atomB->mY;

            float dXAbs = abs(dX);
            float dXAlt = mSimWidth - dXAbs;
            if (dXAlt < dXAbs) {
                dX = dXAlt * (atomA->mX < atomB->mX ? 1.0f : -1.0f);
            }

            float dYAbs = abs(dY);
            float dYAlt = mSimHeight - dYAbs;
            if (dYAlt < dYAbs) {
                dY = dYAlt * (atomA->mY < atomB->mY ? 1.0f : -1.0f);
            }

            if (dX == 0 && dY == 0) {
                continue;
            }

            float d2 = dX * dX + dY * dY;
            if (d2 < 6400) {
                float d = sqrt(d2);
                float f = std::min(1.0f, g / d);
                if (d < atomDiameter) {
                    f += (atomDiameter - d) * 1.0f / atomDiameter;
                }
                fX += f * dX;
                fY += f * dY;
            }
        }
        atomA->mVX = (atomA->mVX + fX * mDt) * mDrag;
        atomA->mVY = (atomA->mVY + fY * mDt) * mDrag;
    }

    for (Atom* atom : mAtoms) {
        atom->mX += atom->mVX * mDt;
        atom->mY += atom->mVY * mDt;

        if (atom->mX < 0) {
            atom->mX += mSimWidth;
        }
        else if (atom->mX >= mSimWidth) {
            atom->mX -= mSimWidth;
        }
        if (atom->mY < 0) {
            atom->mY += mSimHeight;
        }
        else if (atom->mY >= mSimHeight) {
            atom->mY -= mSimHeight;
        }
    }
}

void LifeSimulationHandler::removeAtomType(unsigned int atomTypeId) {
    mAtoms.erase(
        std::remove_if(
            mAtoms.begin(), mAtoms.end(),
            [atomTypeId](Atom* atom) {
                if (atom->getAtomType()->getId() == atomTypeId) {
                    delete atom;
                    return true;
                }
                return false;
            }), mAtoms.end()
                );
    mLSRules.removeAtomType(atomTypeId);
}

void LifeSimulationHandler::shuffleAtomPositions() {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> rangeX(0, mSimWidth);
    std::uniform_real_distribution<float> rangeY(0, mSimWidth);

    for (Atom* atom : mAtoms) {
        atom->mX = rangeX(mt);
        atom->mY = rangeY(mt);
        atom->mVX = 0.0f;
        atom->mVY = 0.0f;
    }
}

void LifeSimulationHandler::shuffleAtomInteractions() {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> range(-1.0f, 1.0f);

    std::vector<AtomType*>& atomTypes = mLSRules.getAtomTypes();

    for (AtomType* atomTypeA : atomTypes) {
        for (AtomType* atomTypeB : atomTypes) {
            mLSRules.setInteraction(atomTypeA->getId(), atomTypeB->getId(), range(mt));
//            mLSRules.setInteraction(atomTypeA.getId(), atomTypeB.getId(), 1.0f);
        }
    }
}

std::vector<Atom*>& LifeSimulationHandler::getAtoms() {
    return mAtoms;
}

LifeSimulationRules& LifeSimulationHandler::getLSRules() {
    return mLSRules;
}
