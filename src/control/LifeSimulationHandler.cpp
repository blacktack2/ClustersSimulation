#include "LifeSimulationHandler.h"

#include <random>

LifeSimulationHandler::LifeSimulationHandler() :
mSimWidth(0), mSimHeight(0), mLSRules(), mAtoms() {
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
    clearSimulation();
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

void LifeSimulationHandler::clearSimulation() {
    for (Atom* atom : mAtoms) {
        delete atom;
    }
    mAtoms.clear();
}

void LifeSimulationHandler::initSimulation() {
    clearSimulation();

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
    for (Atom* atomA : mAtoms) {
        float fX = 0;
        float fY = 0;
        for (Atom* atomB : mAtoms) {
            if (&atomA == &atomB) {
                continue;
            }
            float g = mLSRules.getInteraction(atomA->getAtomType()->getId(), atomB->getAtomType()->getId());
            if (g == 0) {
                continue;
            }

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
                float f = g / d;
                fX += f * dX;
                fY += f * dY;
            }
        }
        atomA->mVX = (atomA->mVX + fX) * 0.5f;
        atomA->mVY = (atomA->mVY + fY) * 0.5f;

        atomA->mX += atomA->mVX;
        atomA->mY += atomA->mVY;

        if (atomA->mX < 0) {
            atomA->mX += mSimWidth;
        } else if (atomA->mX >= mSimWidth) {
            atomA->mX -= mSimWidth;
        }
        if (atomA->mY < 0) {
            atomA->mY += mSimHeight;
        } else if (atomA->mY >= mSimHeight) {
            atomA->mY -= mSimHeight;
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
