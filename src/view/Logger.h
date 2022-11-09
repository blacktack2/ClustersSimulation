#pragma once
#include <filesystem>
#include <fstream>
#include <string>

class Logger {
public:
	~Logger();

	void logMessage(const std::string& message);
	void logWarning(const std::string& message);
	void logError(const std::string& message);

	void logCode(const std::string& code);

	inline bool isValid() { return mIsValid; }

	static Logger& getLogger();
private:
	Logger();

	void log(const std::string& code, const std::string& file, const std::string& message);

	std::ofstream mStream;

	bool mIsValid = false;

	const std::string LOG_FILENAME = "clusters.log";
	const std::string ERROR_FILENAME = "clusters-error.log";
};
