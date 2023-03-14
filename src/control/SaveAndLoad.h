/**
 * @file   SaveAndLoad.h
 * @brief  Utility functions for handling reading/writing to files.
 * 
 * @author Stuart Lewis
 * @date   January 2023
 */
#pragma once
#include <regex>

#include "SimulationHandler.h"

#define MAX_FILE_COUNT 1000
#define CONFIG_FILE_LOCATION "SimConfigs"
#define CONFIG_FILE_EXTENSION "csdat"

/**
 * Find and populate an array of all existing config files.
 * @param files Array of file names to populate.
 * @param count Number of files found.
 */
bool getLoadableFiles(std::string (&files)[MAX_FILE_COUNT], int& count);

/**
 * Save the configuration currently loaded to a new file.
 * @param location File path to save to.
 * @param handler Handler containing the configurations to save.
 */
bool saveToFile(const std::string& location, const SimulationHandler& handler);
/**
 * Load an existing configuration from a file.
 * @param location File path to load from.
 * @param Handler to write the configuration to.
 */
bool loadFromFile(const std::string& location, SimulationHandler& handler);
/**
 * Delete the file at location.
 */
bool deleteFile(const std::string& location);

/**
 * Safe cast a string to a float. Failure will be logged.
 * @returns true if parsing is successful, otherwise false
 */
bool parseFloat(const std::string& s, float& f);
/**
 * Safe cast a string to an unsigned int. Failure will be logged.
 * @returns true if parsing is successful, otherwise false
 */
bool parseUint(const std::string&, unsigned int& i);
