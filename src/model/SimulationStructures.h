#ifndef CLUSTERSSIMULATION_LIFESIMULATIONSTRUCTURES_H
#define CLUSTERSSIMULATION_LIFESIMULATIONSTRUCTURES_H

#include <string>
#include <vector>
#include <optional>
#include <memory>

struct Color {
    float r;
    float g;
    float b;
};

struct vec2 { float x; float y; };

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

Color hslToColor(float h, float s, float l);

#endif //CLUSTERSSIMULATION_LIFESIMULATIONSTRUCTURES_H
