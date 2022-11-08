#include "Logger.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <iomanip>

Logger::Logger() {
	mStream.open(LOG_FILENAME);
	if (!mStream)
		return;
	mStream.close();
	mStream.open(ERROR_FILENAME);
	if (!mStream)
		return;
	mStream.close();
	mIsValid = true;
}

Logger::~Logger() = default;

void Logger::logMessage(std::string message) {
	std::string msg = message.append("\n");
	log("LOG", LOG_FILENAME, msg);
}

void Logger::logWarning(std::string message) {
	std::string msg = message.append("\n");
	log("WARN", LOG_FILENAME, msg);
	log("WARN", ERROR_FILENAME, msg);
}

void Logger::logError(std::string message) {
	std::string msg = message.append("\n");
	log("ERROR", ERROR_FILENAME, msg);
}

void Logger::logCode(std::string code) {
	size_t pos = -1;
	int counter = 1;
	while ((pos = code.find("\n", pos + 1)) != std::string::npos)
		code.replace(pos, 1, "\n {" + std::to_string(++counter) + "} ");
	log("", LOG_FILENAME, std::string(" {1} ").append(code).append("\n"));
}

Logger& Logger::getLogger() {
	static Logger logger;
	return logger;
}

void Logger::log(std::string code, std::string file, std::string message) {
	mStream.open(file, std::ios_base::app);
	if (!mStream)
		return;

	if (code == "") {
		mStream << message;
	} else {
		auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		mStream << "[" << code << "] (" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << ") | " << message;
	}
	mStream.close();
}
