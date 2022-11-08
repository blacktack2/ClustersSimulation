#pragma once
#include <filesystem>
#include <fstream>
#include <string>

class Logger {
public:
	~Logger();

	void logMessage(std::string message);
	void logWarning(std::string message);
	void logError(std::string message);

	void logCode(std::string code);

	inline bool isValid() { return mIsValid; }

	static Logger& getLogger();
private:
	Logger();

	void log(std::string code, std::string file, std::string message);

	std::ofstream mStream;

	bool mIsValid = false;

	const std::string LOG_FILENAME = "clusters.log";
	const std::string ERROR_FILENAME = "clusters-error.log";
};
