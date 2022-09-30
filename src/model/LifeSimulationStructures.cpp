#include "LifeSimulationStructures.h"

#include <random>

static unsigned int idCounter = 0;

AtomType::AtomType() :
mId(idCounter++), mColor({0.0f, 0.0f, 0.0f}), mQuantity(200u), mFriendlyName(std::to_string(mId)) {

}

AtomType::~AtomType() {

}

unsigned int AtomType::getId() const {
    return mId;
}

Color AtomType::getColor() {
    return mColor;
}

void AtomType::setColor(Color color) {
    mColor = color;
}

void AtomType::setColorR(float r) {
    mColor.r = std::min(std::max(r, 0.0f), 1.0f);
}

void AtomType::setColorG(float g) {
    mColor.g = std::min(std::max(g, 0.0f), 1.0f);
}

void AtomType::setColorB(float b) {
    mColor.b = std::min(std::max(b, 0.0f), 1.0f);
}

unsigned int AtomType::getQuantity() const {
    return mQuantity;
}

void AtomType::setQuantity(unsigned int quantity) {
    mQuantity = quantity;
}

std::string AtomType::getFriendlyName() {
    return mFriendlyName;
}

void AtomType::setFriendlyName(std::string friendlyName) {
    mFriendlyName = std::move(friendlyName);
}

Atom::Atom(AtomType* atomType) :
mAtomType(atomType), mX(0.0f), mY(0.0f), mVX(0.0f), mVY(0.0f){

}

AtomType* Atom::getAtomType() {
    return mAtomType;
}

LifeSimulationRules::LifeSimulationRules() :
mAtomRadius(3.0f), mAtomTypes(), mInteractions() {

}

LifeSimulationRules::~LifeSimulationRules() {
    clear();
}

void LifeSimulationRules::clear() {
    for (AtomType* atomType : mAtomTypes) {
        delete atomType;
    }
    mAtomTypes.clear();
    mInteractions.clear();
}

AtomType* LifeSimulationRules::newAtomType() {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> rangeH(0.0f, 360.0f);
    std::uniform_real_distribution<float> rangeS(1.0f, 1.0f);
    std::uniform_real_distribution<float> rangeL(0.5f, 0.5f);

    AtomType* atomType = new AtomType();
    atomType->setColor(hslToColor(rangeH(mt), rangeS(mt), rangeL(mt)));
    
    return addAtomType(atomType);
}

AtomType* LifeSimulationRules::addAtomType(AtomType* atomType) {
    for (AtomType* atomType2 : mAtomTypes) {
        InteractionSet is1 = {atomType2->getId(), atomType->getId(), 0};
        InteractionSet is2 = {atomType->getId(), atomType2->getId(), 0};
        mInteractions.push_back(is1);
        mInteractions.push_back(is2);
    }
    InteractionSet interactionSet = {atomType->getId(), atomType->getId(), 0};
    mInteractions.push_back(interactionSet);
    mAtomTypes.push_back(atomType);
    return atomType;
}

AtomType* LifeSimulationRules::getAtomType(unsigned int atomTypeId) {
    for (AtomType* atomType : mAtomTypes) {
        if (atomType->getId() == atomTypeId) {
            return atomType;
        }
    }
    return nullptr;
}

void LifeSimulationRules::removeAtomType(unsigned int atomTypeId) {
    mInteractions.erase(
        std::remove_if(
            mInteractions.begin(), mInteractions.end(),
            [atomTypeId](InteractionSet is) {
                return is.aId == atomTypeId || is.bId == atomTypeId;
            }), mInteractions.end()
                );
    mAtomTypes.erase(
        std::remove_if(
            mAtomTypes.begin(), mAtomTypes.end(),
            [atomTypeId](AtomType* atomType) {
                return atomType->getId() == atomTypeId;
            }), mAtomTypes.end()
                );
}

std::vector<AtomType*>& LifeSimulationRules::getAtomTypes() {
    return mAtomTypes;
}

void LifeSimulationRules::setInteraction(unsigned int aId, unsigned int bId, float interaction) {
    for (auto & mInteraction : mInteractions) {
        if (mInteraction.aId == aId && mInteraction.bId == bId) {
            mInteraction.value = interaction;
            return;
        }
    }
}

float LifeSimulationRules::getInteraction(unsigned int aId, unsigned int bId) {
    for (auto & interaction : mInteractions) {
        if (interaction.aId == aId && interaction.bId == bId) {
            return interaction.value;
        }
    }
    return 0.0f;
}

void LifeSimulationRules::setAtomRadius(float atomRadius) {
    mAtomRadius = atomRadius;
}

float LifeSimulationRules::getAtomRadius() {
    return mAtomRadius;
}

Color LifeSimulationRules::hslToColor(float h, float s, float l) {
    float c = (1 - std::abs(2 * l - 1)) * s;
    float x = c * (1 - std::abs(std::fmodf(h / 60, 2) - 1));
    float m = l - c / 2;

    float r;
    float g;
    float b;

    if (0 <= h && h < 60) {
        r = c;
        g = x;
        b = 0;
    } else if (60 <= h && h < 120) {
        r = x;
        g = c;
        b = 0;
    } else if (120 <= h && h < 180) {
        r = 0;
        g = c;
        b = x;
    } else if (180 <= h && h < 240) {
        r = 0;
        g = x;
        b = c;
    } else if (240 <= h && h < 300) {
        r = x;
        g = 0;
        b = c;
    } else {
        r = c;
        g = 0;
        b = x;
    }

    return {r + m, g + m, b + m};
}
