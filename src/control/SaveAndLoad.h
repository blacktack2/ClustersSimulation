#ifndef LIFESIMULATIONC_SAVEANDLOAD_H
#define LIFESIMULATIONC_SAVEANDLOAD_H

#include <regex>

#include "SimulationHandler.h"

#define MAX_FILE_COUNT 1000
#define CONFIG_FILE_LOCATION "SimConfigs"
#define CONFIG_FILE_EXTENSION "csdat"

bool getLoadableFiles(std::string (&files)[MAX_FILE_COUNT], int& count);

bool saveToFile(const std::string& location, const SimulationHandler& handler);
bool loadFromFile(const std::string& location, SimulationHandler& handler);

bool parseFloat(const std::string& s, float& f);
bool parseUint(const std::string&, unsigned int& i);

#endif //LIFESIMULATIONC_SAVEANDLOAD_H