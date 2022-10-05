#ifndef LIFESIMULATIONC_SAVEANDLOAD_H
#define LIFESIMULATIONC_SAVEANDLOAD_H

#include <regex>

#include "LifeSimulationHandler.h"

#define MAX_FILE_COUNT 1000
#define CONFIG_FILE_LOCATION "SimConfigs"
#define CONFIG_FILE_EXTENSION "csdat"
const std::regex CONFIG_FILE_REGEX = std::regex(CONFIG_FILE_LOCATION + std::string("[/\\\\]([a-zA-Z0-9_-]+)\\.") + CONFIG_FILE_EXTENSION);

bool getLoadableFiles(std::string (&files)[MAX_FILE_COUNT], int& count);

bool saveToFile(std::string location, LifeSimulationHandler& handler);
bool loadFromFile(std::string location, LifeSimulationHandler& handler);

bool parseFloat(std::string s, float& f);
bool parseUint(std::string, unsigned int& i);

#endif //LIFESIMULATIONC_SAVEANDLOAD_H