#pragma once
#include "../../glm/vec3.hpp"

#include <string>
#include <vector>
#include <memory>

struct AtomType {
    AtomType();
    explicit AtomType(unsigned int id);
    unsigned int id;

    float r;
    float g;
    float b;

    unsigned int quantity;
    std::string friendlyName;
};

struct AtomTypeRaw {
    explicit AtomTypeRaw();
    explicit AtomTypeRaw(const AtomType& at);
    float r;
    float g;
    float b;
};

struct Atom {
    Atom();
    explicit Atom(unsigned int atomType);

    float x, y, vx, vy, fx, fy;

    unsigned int atomType;
};

glm::vec3 hslToColor(float h, float s, float l);
