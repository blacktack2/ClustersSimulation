#pragma once
#include "../../glm/vec3.hpp"

#include <string>
#include <vector>
#include <memory>

typedef unsigned int atom_type_id;

struct AtomType {
    AtomType();
    explicit AtomType(atom_type_id id);
    atom_type_id id;

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
    explicit Atom(atom_type_id atomType);

    float x, y, vx, vy, fx, fy;

    atom_type_id atomType;
};

glm::vec3 hslToColor(float h, float s, float l);
