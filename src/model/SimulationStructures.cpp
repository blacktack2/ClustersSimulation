#include "SimulationStructures.h"

#include <algorithm>
#include <cmath>
#include <random>
#ifndef ITERATE_ON_COMPUTE_SHADER
#include <memory>
#endif

static unsigned int idCounter = 0;

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
