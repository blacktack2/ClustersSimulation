#include "SimulationStructures.h"

#include <algorithm>
#include <cmath>
#include <random>
#ifndef ITERATE_ON_COMPUTE_SHADER
#include <memory>
#endif

static unsigned int idCounter = 0;

#ifdef ITERATE_ON_COMPUTE_SHADER
AtomType::AtomType() :
    id(idCounter++), r(0.0f), g(0.0f), b(0.0f), friendlyName(std::to_string(id)), quantity(200) {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> rangeH(0.0f, 360.0f);
    std::uniform_real_distribution<float> rangeS(1.0f, 1.0f);
    std::uniform_real_distribution<float> rangeL(0.5f, 0.5f);

    Color color = hslToColor(rangeH(mt), rangeS(mt), rangeL(mt));
    r = color.r;
    g = color.g;
    b = color.b;
}

AtomType::AtomType(unsigned int id_) :
id(id_), r(0.0f), g(0.0f), b(0.0f), friendlyName(std::to_string(id)), quantity(200) {
    idCounter = std::max(idCounter, id + 1);

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> rangeH(0.0f, 360.0f);
    std::uniform_real_distribution<float> rangeS(1.0f, 1.0f);
    std::uniform_real_distribution<float> rangeL(0.5f, 0.5f);

    Color color = hslToColor(rangeH(mt), rangeS(mt), rangeL(mt));
    r = color.r;
    g = color.g;
    b = color.b;
}

AtomTypeRaw::AtomTypeRaw() :
r(0.0f), g(0.0f), b(0.0f) {
}

AtomTypeRaw::AtomTypeRaw(const AtomType& at) :
r(at.r), g(at.g), b(at.b) {
}

Atom::Atom() :
atomType(0), x(0), y(0), vx(0), vy(0), fx(0), fy(0) {

}

Atom::Atom(unsigned int atomType_) :
atomType(atomType_), x(0), y(0), vx(0), vy(0), fx(0), fy(0) {

}
#else
Atom::Atom(AtomType* atomType) :
mAtomType(atomType), mX(0.0f), mY(0.0f), mVX(0.0f), mVY(0.0f), mFX(0.0f), mFY(0.0f) {

}

AtomType::AtomType() :
mId(idCounter++), mColor({0.0f, 0.0f, 0.0f}), mQuantity(200u),
mFriendlyName(std::to_string(mId)) {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> rangeH(0.0f, 360.0f);
    std::uniform_real_distribution<float> rangeS(1.0f, 1.0f);
    std::uniform_real_distribution<float> rangeL(0.5f, 0.5f);
    
    mColor = hslToColor(rangeH(mt), rangeS(mt), rangeL(mt));
}

AtomType::AtomType(unsigned int id) :
mId(id), mColor({0.0f, 0.0f, 0.0f}), mQuantity(200u),
mFriendlyName(std::to_string(mId)) {
    idCounter = std::max(mId + 1, idCounter);

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> rangeH(0.0f, 360.0f);
    std::uniform_real_distribution<float> rangeS(1.0f, 1.0f);
    std::uniform_real_distribution<float> rangeL(0.5f, 0.5f);

    mColor = hslToColor(rangeH(mt), rangeS(mt), rangeL(mt));
}

AtomType::AtomType(const AtomType& other) :
mId(other.mId), mColor(other.mColor), mQuantity(other.mQuantity),
mFriendlyName(other.mFriendlyName) {
    idCounter = std::max(mId + 1, idCounter);
}

AtomType::~AtomType() = default;

const unsigned int& AtomType::getId() const {
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

SimulationRules::SimulationRules() :
mAtomRadius(3.0f), mAtomTypes(), mInteractions() {

}

SimulationRules::~SimulationRules() = default;

void SimulationRules::clearAtomTypes() {
    mAtomTypes.clear();
    mInteractions.clear();
}

AtomType* SimulationRules::newAtomType() {
    AtomType* at = mAtomTypes.emplace_back(std::make_unique<AtomType>()).get();
    createInteractionsForAtomType(at);
    return at;
}

AtomType* SimulationRules::newAtomType(unsigned int id) {
    AtomType* exists = getAtomType(id);
    if (exists != nullptr) {
        return exists;
    }

    AtomType* at = mAtomTypes.emplace_back(std::make_unique<AtomType>(id)).get();
    createInteractionsForAtomType(at);
    return at;
}

AtomType* SimulationRules::getAtomType(unsigned int atomTypeId) const {
    for (const std::unique_ptr<AtomType>& atomType : mAtomTypes) {
        if (atomType->getId() == atomTypeId) {
            return atomType.get();
        }
    }
    return nullptr;
}

void SimulationRules::removeAtomType(unsigned int atomTypeId) {
    mInteractions.erase(
        std::remove_if(
            mInteractions.begin(), mInteractions.end(),
            [atomTypeId](InteractionSet& is) {
                return is.aId == atomTypeId || is.bId == atomTypeId;
            }), mInteractions.end()
                );
    mAtomTypes.erase(
        std::remove_if(
            mAtomTypes.begin(), mAtomTypes.end(),
            [atomTypeId](std::unique_ptr<AtomType>& atomType) {
                return atomType->getId() == atomTypeId;
            }), mAtomTypes.end()
                );
}

const std::vector<std::unique_ptr<AtomType>>& SimulationRules::getAtomTypes() const {
    return mAtomTypes;
}

int SimulationRules::getAtomCount() const {
    int count = 0;
    for (const std::unique_ptr<AtomType>& atomType : mAtomTypes) {
        count += atomType->getQuantity();
    }
    return count;
}

unsigned int SimulationRules::getAtomTypeCount() const {
    return mAtomTypes.size();
}

void SimulationRules::clearInteractions() {
    for (InteractionSet& is : mInteractions) {
        is.value = 0.0f;
    }
}

void SimulationRules::setInteraction(unsigned int aId, unsigned int bId, float interaction) {
    for (InteractionSet& is : mInteractions) {
        if (is.aId == aId && is.bId == bId) {
            is.value = interaction;
            return;
        }
    }
}

float SimulationRules::getInteraction(unsigned int aId, unsigned int bId) const {
    for (const InteractionSet& is : mInteractions) {
        if (is.aId == aId && is.bId == bId) {
            return is.value;
        }
    }
    return 0.0f;
}

std::vector<InteractionSet>& SimulationRules::getInteractions() {
    return mInteractions;
}

void SimulationRules::setAtomRadius(float atomRadius) {
    mAtomRadius = atomRadius;
}

const float& SimulationRules::getAtomRadius() const {
    return mAtomRadius;
}

void SimulationRules::createInteractionsForAtomType(AtomType* atomType) {
    for (std::unique_ptr<AtomType>& atomType2 : mAtomTypes) {
        if (atomType == atomType2.get()) {
            continue;
        }
        InteractionSet is1 = {atomType2->getId(), atomType->getId(), 0};
        InteractionSet is2 = {atomType->getId(), atomType2->getId(), 0};
        mInteractions.push_back(is1);
        mInteractions.push_back(is2);
    }
    InteractionSet interactionSet = {atomType->getId(), atomType->getId(), 0};
    mInteractions.push_back(interactionSet);
}

#endif

Color hslToColor(float h, float s, float l) {
    float c = (1 - std::abs(2 * l - 1)) * s;
    float x = c * (1 - std::abs(fmodf(h / 60, 2) - 1));
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
