#include "SaveAndLoad.h"

#include <filesystem>
#include <fstream>
#include <map>
#include <string>

const std::regex& getConfigFileRegex() {
    static const std::regex CONFIG_FILE_REGEX(CONFIG_FILE_LOCATION + std::string(R"([/\\]([a-zA-Z0-9_-]+)\.)") + CONFIG_FILE_EXTENSION);
    return CONFIG_FILE_REGEX;
}

bool getLoadableFiles(std::string (&files)[MAX_FILE_COUNT], int& count) {
	std::filesystem::create_directory(CONFIG_FILE_LOCATION);

	count = 0;
	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(CONFIG_FILE_LOCATION)) {
		std::string filepath = std::string(entry.path().u8string());
		std::smatch matches;
		if (std::regex_search(filepath, matches, getConfigFileRegex())) {
			files[count++] = matches[1];
			if (count >= MAX_FILE_COUNT) {
				break;
			}
		}
	}

	return true;
}

bool saveToFile(const std::string& location, const SimulationHandler& handler) {
	std::string data;

	data += "Width:" + std::to_string(handler.getWidth()) + " Height:" + std::to_string(handler.getHeight()) + "\n";
	data += "DT:" + std::to_string(handler.getDt()) + "\n";
	data += "Drag:" + std::to_string(handler.getDrag()) + "\n";
	data += "Range:" + std::to_string(handler.getInteractionRange()) + "\n";
	data += "CollisionForce:" + std::to_string(handler.getCollisionForce()) + "\n";
	data += "AtomDiameter:" + std::to_string(handler.getAtomDiameter()) + "\n";
	data += "StartCondition:" + std::to_string(handler.startCondition) + "\n";

	std::vector<unsigned int> atomTypeIds = handler.getAtomTypeIds();
	for (unsigned int atomTypeId : atomTypeIds) {
		std::string name = handler.getAtomTypeFriendlyName(atomTypeId);
		unsigned int quantity = handler.getAtomTypeQuantity(atomTypeId);
		data += "ID:" + std::to_string(atomTypeId) + " Name:" + name + " Quantity:" + std::to_string(quantity) + " ";
		Color color = handler.getAtomTypeColor(atomTypeId);
		data += "R:" + std::to_string(color.r) + " G:" + std::to_string(color.g) + " B:" + std::to_string(color.b) + "\n";
	}

	for (unsigned int aId : atomTypeIds) {
		for (unsigned int bId : atomTypeIds) {
			data += "Aid:" + std::to_string(aId) + " Bid:" + std::to_string(bId) + " Value:" + std::to_string(handler.getInteraction(aId, bId)) + "\n";
		}
	}

	std::ofstream file;
	file.open(location);
	if (!file) {
		fprintf(stderr, "Failed to open file %s!\n", location.c_str());
		return false;
	}
	file << data;
	file.close();

	return true;
}

bool loadFromFile(const std::string& location, SimulationHandler& handler) {
	static const std::regex atomTypeRegex = std::regex("^ID:([0-9]+) Name:([A-Za-z0-9_-]*) Quantity:([0-9]+) R:([0-9]+(\\.[0-9]+)?) G:([0-9]+(\\.[0-9]+)?) B:([0-9]+(\\.[0-9]+)?)\r?$");
	static const std::regex interactionRegex = std::regex("^Aid:([0-9]+) Bid:([0-9]+) Value:(-?[0-9]+(\\.[0-9]+)?)\r?$");
	static const std::regex sizeRegex = std::regex("^Width:([0-9]+(\\.[0-9]+)?) Height:([0-9]+(\\.[0-9]+)?)\r?$");
	static const std::regex dtRegex = std::regex("^DT:([0-9]+(\\.[0-9]+)?)\r?$");
	static const std::regex dragRegex = std::regex("^Drag:([0-9]+(\\.[0-9]+)?)\r?$");
	static const std::regex interactionRangeRegex = std::regex("^Range:([0-9]+(\\.[0-9]+)?)\r?$");
	static const std::regex collisionForceRegex = std::regex("^CollisionForce:([0-9]+(\\.[0-9]+)?)\r?$");
	static const std::regex atomDiameterRegex = std::regex("^AtomDiameter:([0-9]+(\\.[0-9]+)?)\r?$");
	static const std::regex startConditionRegex = std::regex("^StartCondition:([0-9]+)\r?$");

	handler.clearAtomTypes();
	handler.setBounds(1000.0f, 1000.0f);
	handler.setDt(1.0f);
	handler.setDrag(0.5f);
	handler.setInteractionRange(80.0f);
	handler.setCollisionForce(1.0f);
	handler.setAtomDiameter(3.0f);

	handler.startCondition = StartConditionRandom;

	std::string line;
	std::ifstream file;
	file.open(location);

	if (!file.is_open()) {
		fprintf(stderr, "Failed to open file %s!\n", location.c_str());
		return false;
	}
	std::vector<std::string> atomTypes{};
	std::vector<std::string> interactions{};
	while (getline(file, line)) {
		std::smatch matches;
		if (std::regex_match(line, atomTypeRegex)) {
			atomTypes.push_back(line);
		} else if (std::regex_match(line, interactionRegex)) {
			interactions.push_back(line);
		} else if (std::regex_search(line, matches, sizeRegex)) {
			float width;
			float height;
			if (!parseFloat(matches[1], width) || !parseFloat(matches[3], height)) {
				fprintf(stderr, "Failed to parse float! Line: %s", matches.str(0).c_str());
			} else {
				handler.setBounds(width, height);
			}
		} else if (std::regex_search(line, matches, dtRegex)) {
			float dt;
			if (!parseFloat(matches[1], dt)) {
				fprintf(stderr, "Failed to parse float! Line: %s", matches.str(0).c_str());
			} else {
				handler.setDt(dt);
			}
		} else if (std::regex_search(line, matches, dragRegex)) {
			float drag;
			if (!parseFloat(matches[1], drag)) {
				fprintf(stderr, "Failed to parse float! Line: %s", matches.str(0).c_str());
			} else {
				handler.setDrag(drag);
			}
		} else if (std::regex_search(line, matches, interactionRangeRegex)) {
			float interactionRange;
			if (!parseFloat(matches[1], interactionRange)) {
				fprintf(stderr, "Failed to parse float! Line: %s", matches.str(0).c_str());
			} else {
				handler.setInteractionRange(interactionRange);
			}
		} else if (std::regex_search(line, matches, collisionForceRegex)) {
			float collisionForce;
			if (!parseFloat(matches[1], collisionForce)) {
				fprintf(stderr, "Failed to parse float! Line: %s", matches.str(0).c_str());
			} else {
				handler.setCollisionForce(collisionForce);
			}
		} else if (std::regex_search(line, matches, atomDiameterRegex)) {
			float atomDiameter;
			if (!parseFloat(matches[1], atomDiameter)) {
				fprintf(stderr, "Failed to parse float! Line: %s", matches.str(0).c_str());
			} else {
				handler.setAtomDiameter(atomDiameter);
			}
		} else if (std::regex_search(line, matches, startConditionRegex)) {
			unsigned int startCondition;
			if (!parseUint(matches[1], startCondition)) {
				fprintf(stderr, "Failed to parse float! Line: %s", matches.str(0).c_str());
			} else {
				handler.startCondition = (StartCondition) startCondition;
			}
		}
	}
	file.close();

	std::map<unsigned int, unsigned int> idMap;
	for (auto& l : atomTypes) {
		std::smatch matches;
		if (std::regex_search(l, matches, atomTypeRegex)) {
			unsigned int id;
			std::string name = matches[2];
			unsigned int quantity;
			float r;
			float g;
			float b;
			if (!parseUint(matches[1], id) || !parseUint(matches[3], quantity) ||
				!parseFloat(matches[4], r) || !parseFloat(matches[6], g) || !parseFloat(matches[8], b)) {
				fprintf(stderr, "Failed to parse AtomType values! Line: %s", matches.str(0).c_str());
			} else {
				unsigned int newId = handler.newAtomType();
				idMap.emplace(id, newId);
				handler.setAtomTypeFriendlyName(newId, name);
				handler.setAtomTypeQuantity(newId, quantity);
				handler.setAtomTypeColor(newId, { r, g, b });
			}
		}
	}

	for (auto& l : interactions) {
		std::smatch matches;
		if (std::regex_search(l, matches, interactionRegex)) {
			unsigned int aId;
			unsigned int bId;
			float value;
			if (!parseUint(matches[1], aId) || !parseUint(matches[2], bId) || !parseFloat(matches[3], value)) {
				fprintf(stderr, "Failed to parse interaction values! Line: %s", matches.str(0).c_str());
			} else {
				handler.setInteraction(idMap[aId], idMap[bId], value);
			}
		}
	}

	return true;
}

bool deleteFile(const std::string& location) {
	try {
		if (!std::filesystem::remove(location)) {
			fprintf(stderr, "Failed to find file %s!\n", location.c_str());
			return false;
		}
	} catch(const std::filesystem::filesystem_error& err) {
		fprintf(stderr, "Filesystem error: %s", err.what());
		return false;
	}
	return true;;
}

bool parseFloat(const std::string& s, float& f) {
	std::size_t pos{};
	try {
		f = std::stof(s, &pos);
	} catch (std::invalid_argument const& ex) {
		fprintf(stderr, "std::invalid_argument::what():%s\n", ex.what());
		return false;
	} catch (std::out_of_range const& ex) {
		const long long ll {std::stoll(s, &pos)};
		fprintf(stderr, "std::out_of_range::what():%s\nstd::stoll('%llu'); pos: %lu\n", ex.what(), ll, pos);
		return false;
	}
	return true;
}

bool parseUint(const std::string& s, unsigned int& i) {
	std::size_t pos{};
	try {
		unsigned long l = std::stoul(s, &pos);
		if (l > std::numeric_limits<unsigned int>::max()) {
			fprintf(stderr, "Uint value out of range: %s", s.c_str());
		}
		i = l;
	} catch (std::invalid_argument const& ex) {
		fprintf(stderr, "std::invalid_argument::what():%s\n", ex.what());
		return false;
	} catch (std::out_of_range const& ex) {
		const long long ll {std::stoll(s, &pos)};
		fprintf(stderr, "std::out_of_range::what():%s\nstd::stoll(\'%llu\'); pos: %lu\n", ex.what(), ll, pos);
		return false;
	}
	return true;
}
