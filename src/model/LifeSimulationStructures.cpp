#include "LifeSimulationStructures.h"

static uint idCounter = 0;

AtomType::AtomType() : mId(idCounter++) {
    mColor = {0x00, 0x00, 0x00, 0xFF};
    mQuantity = 500u;

    mFriendlyName = std::to_string(mId);
}

AtomType::~AtomType() {

}

uint AtomType::getId() const {
    return mId;
}

SDL_Color AtomType::getColor() {
    return mColor;
}

void AtomType::setColor(SDL_Color color) {
    mColor = color;
}

uint AtomType::getQuantity() const {
    return mQuantity;
}

void AtomType::setQuantity(uint quantity) {
    mQuantity = quantity;
}

std::string AtomType::getFriendlyName() {
    return mFriendlyName;
}

void AtomType::setFriendlyName(std::string friendlyName) {
    mFriendlyName = std::move(friendlyName);
}

Atom::Atom(AtomType* atomType) {
    mAtomType = atomType;

    mX = 0.0f;
    mY = 0.0f;

    mVX = 0.0f;
    mVY = 0.0f;
}

AtomType *Atom::getAtomType() {
    return mAtomType;
}

LifeSimulationRules::LifeSimulationRules() {
    mAtomTypes.clear();
}

LifeSimulationRules::~LifeSimulationRules() {
    clear();
}

void LifeSimulationRules::clear() {
    for (auto & atomType : mAtomTypes) {
        delete atomType;
    }
    mAtomTypes.clear();
    mInteractions.clear();
}

void LifeSimulationRules::addAtomType(AtomType *atomType) {
    for (auto & mAtomType : mAtomTypes) {
        InteractionSet is1 = {mAtomType->getId(), atomType->getId(), 0};
        InteractionSet is2 = {atomType->getId(), mAtomType->getId(), 0};
        mInteractions.push_back(is1);
        mInteractions.push_back(is2);
    }
    InteractionSet is = {atomType->getId(), atomType->getId(), 0};
    mInteractions.push_back(is);
    mAtomTypes.push_back(atomType);
}

AtomType *LifeSimulationRules::getAtomType(uint atomTypeId) {
    for (auto & atomType : mAtomTypes) {
        if (atomType->getId() == atomTypeId) {
            return atomType;
        }
    }
    return nullptr;
}

void LifeSimulationRules::removeAtomType(uint atomTypeId) {
    std::vector<uint> typesToRemove;
    std::vector<uint> interactionsToRemove;
    for (uint i = 0; i < mAtomTypes.size(); i++) {
        if (mAtomTypes[i]->getId() == atomTypeId) {
            typesToRemove.push_back(i);
        }
    }
    for (uint i = 0; i < mInteractions.size(); i++) {
        if (mInteractions[i].aId == atomTypeId || mInteractions[i].bId == atomTypeId) {
            interactionsToRemove.push_back(i);
        }
    }
    for (uint i : typesToRemove) {
        mAtomTypes.erase(mAtomTypes.begin() + i);
    }
    for (uint i : interactionsToRemove) {
        mInteractions.erase(mInteractions.begin() + i);
    }
}

std::vector<AtomType*>* LifeSimulationRules::getAtomTypes() {
    return &mAtomTypes;
}

void LifeSimulationRules::setInteraction(uint aId, uint bId, float interaction) {
    for (auto & mInteraction : mInteractions) {
        if (mInteraction.aId == aId && mInteraction.bId == bId) {
            mInteraction.value = interaction;
            return;
        }
    }
}

float LifeSimulationRules::getInteraction(uint aId, uint bId) {
    for (auto & interaction : mInteractions) {
        if (interaction.aId == aId && interaction.bId == bId) {
            return interaction.value;
        }
    }
    return 0.0f;
}
