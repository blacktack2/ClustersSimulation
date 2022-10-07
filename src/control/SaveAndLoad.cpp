#include "SaveAndLoad.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

bool getLoadableFiles(std::string (&files)[MAX_FILE_COUNT], int& count) {
	std::filesystem::create_directory(CONFIG_FILE_LOCATION);

	count = 0;
	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(CONFIG_FILE_LOCATION)) {
		std::string filepath = std::string(entry.path().u8string());
		std::smatch matches;
		if (std::regex_search(filepath, matches, CONFIG_FILE_REGEX)) {
			files[count++] = matches[1];
			if (count >= MAX_FILE_COUNT) {
				break;
			}
		}
	}

	return true;
}

bool saveToFile(std::string location, SimulationHandler& handler) {
	std::string data = "";

	data += "Width:" + std::to_string(handler.getWidth()) + " Height:" + std::to_string(handler.getHeight()) + "\n";
	data += "DT:" + std::to_string(handler.getDt()) + "\n";
	data += "Drag:" + std::to_string(handler.getDrag()) + "\n";
	data += "Range:" + std::to_string(handler.getInteractionRange()) + "\n";

	for (AtomType& atomType : handler.getLSRules().getAtomTypes()) {
		data += "ID:" + std::to_string(atomType.getId()) + " Name:" + atomType.getFriendlyName() + " Quantity:" + std::to_string(atomType.getQuantity()) + " ";
		Color c = atomType.getColor();
		data += "R:" + std::to_string(c.r) + " G:" + std::to_string(c.g) + " B:" + std::to_string(c.b) + "\n";
	}

	for (InteractionSet is : handler.getLSRules().getInteractions()) {
		data += "Aid:" + std::to_string(is.aId) + " Bid:" + std::to_string(is.bId) + " Value:" + std::to_string(is.value) + "\n";
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

bool loadFromFile(std::string location, SimulationHandler& handler) {
	static const std::regex atomTypeRegex = std::regex("^ID:([0-9]+) Name:([A-Za-z0-9_-]+) Quantity:([0-9]+) R:([0-9]+(\\.[0-9]+)?) G:([0-9]+(\\.[0-9]+)?) B:([0-9]+(\\.[0-9]+)?)$");
	static const std::regex interactionRegex = std::regex("^Aid:([0-9]+) Bid:([0-9]+) Value:(-?[0-9]+(\\.[0-9]+)?)$");
	static const std::regex sizeRegex = std::regex("^Width:([0-9]+(\\.[0-9]+)?) Height:([0-9]+(\\.[0-9]+)?)$");
	static const std::regex dtRegex = std::regex("^DT:([0-9]+(\\.[0-9]+)?)$");
	static const std::regex dragRegex = std::regex("^Drag:([0-9]+(\\.[0-9]+)?)$");
	static const std::regex interactionRangeRegex = std::regex("^Range:([0-9]+(\\.[0-9]+)?)$");

	SimulationRules& rules = handler.getLSRules();
	handler.clearAtomTypes();

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
		}
	}
	file.close();

	for (std::string l : atomTypes) {
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
				AtomType& at = rules.newAtomType(id);
				at.setFriendlyName(name);
				at.setQuantity(quantity);
				at.setColor({r, g, b});
			}
		}
	}

	for (std::string l : interactions) {
		std::smatch matches;
		if (std::regex_search(l, matches, interactionRegex)) {
			unsigned int aId;
			unsigned int bId;
			float value;
			if (!parseUint(matches[1], aId) || !parseUint(matches[2], bId) || !parseFloat(matches[3], value)) {
				fprintf(stderr, "Failed to parse interaction values! Line: %s", matches.str(0).c_str());
			} else {
				rules.setInteraction(aId, bId, value);
			}
		}
	}

	return true;
}

bool parseFloat(std::string s, float& f) {
	std::size_t pos{};
	try {
		f = std::stof(s, &pos);
	} catch (std::invalid_argument const& ex) {
		fprintf(stderr, "std::invalid_argument::what():%s\n", ex.what());
		return false;
	} catch (std::out_of_range const& ex) {
		const long long ll {std::stoll(s, &pos)};
		fprintf(stderr, "std::out_of_range::what():%s\nstd::stoll('%ll'); pos: %zu\n", ex.what(), ll, pos);
		return false;
	}
	return true;
}

bool parseUint(std::string s, unsigned int& i) {
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
		fprintf(stderr, "std::out_of_range::what():%s\nstd::stoll('%ll'); pos: %zu\n", ex.what(), ll, pos);
		return false;
	}
	return true;
}
