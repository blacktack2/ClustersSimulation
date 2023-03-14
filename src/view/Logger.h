/**
 * @file   Logger.h
 * @brief  Logging class for handling logging to a file.
 * 
 * @author Stuart Lewis
 * @date   January 2023
 */
#pragma once
#include <filesystem>
#include <fstream>
#include <string>

/**
 * Logging class for logging debug information to a file.
 */
class Logger {
public:
	~Logger();

	/**
	 * Log trace messages
	 */
	void logMessage(const std::string& message);
	/**
	 * Log warning messages
	 */
	void logWarning(const std::string& message);
	/**
	 * Log error messages
	 */
	void logError(const std::string& message);
	/**
	 * Log multiline code snippets.
	 */
	void logCode(const std::string& code);

	/**
	 * @returns false if either of the log or error files cannot be opened,
	 * otherwise true.
	 */
	inline bool isValid() { return mIsValid; }

	/**
	 * Singleton getter.
	 */
	static Logger& getLogger();
private:
	/**
	 * Private constructor, should only be called once (see Logger::getLogger).
	 */
	Logger();

	void log(const std::string& code, const std::string& file, const std::string& message);

	std::ofstream mStream;

	bool mIsValid = false;

	const std::string LOG_FILENAME = "clusters.log";
	const std::string ERROR_FILENAME = "clusters-error.log";
};
