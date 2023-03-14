/**
 * @file   SimulationStructures.h
 * @brief  Data structures for representing components of the simulation.
 * 
 * @author Stuart Lewis
 * @date   January 2023
 */
#pragma once
#include "../../glm/vec3.hpp"

#include <string>
#include <vector>
#include <memory>

typedef unsigned int atom_type_id;

/**
 * Classification structure for groups of Atoms which can have varying
 * interactions with other groups.
 */
struct AtomType {
    /**
     * Construct and auto-assign a unique id. Colour is generated randomly with
     * high saturation.
     */
    AtomType();
    /**
     * Construct with pre-defined unique id. Colour is generated randomly with
     * high saturation
     */
    explicit AtomType(atom_type_id id);
    atom_type_id id;

    float r;
    float g;
    float b;

    unsigned int quantity;
    std::string friendlyName;
};

/**
 * Minimum data structure for AtomTypes, primarily for sending directly to the
 * GPU. See AtomType.
 */
struct AtomTypeRaw {
    /**
     * Construct with default black (#000000) colour.
     */
    explicit AtomTypeRaw();
    /**
     * Construct with matching parameters.
     * @param at AtomType to match the parameters of.
     */
    explicit AtomTypeRaw(const AtomType& at);
    float r;
    float g;
    float b;
};

/**
 * Classification structure for individual Atoms.
 */
struct Atom {
    /**
     * Construct with zeroed parameters and pointing to AtomType at index 0;
     */
    Atom();
    /**
     * Construct with zeroed parameters.
     * @param atomType AtomType to classify this Atom under.
     */
    explicit Atom(atom_type_id atomType);

    float x, y, vx, vy, fx, fy;

    /** Indexable position of the AtomType this Atom is classified under. */
    atom_type_id atomType;
};

glm::vec3 hslToColor(float h, float s, float l);
