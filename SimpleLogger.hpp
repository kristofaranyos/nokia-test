#ifndef SIMPLELOGGER_HPP
#define SIMPLELOGGER_HPP


#include <cstdint>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>

/*
 * Using a solution from https://stackoverflow.com/questions/24365331/how-can-i-generate-uuid-in-c-without-using-boost-library
 * Copy pasting may seem bad, but there is no need to reinvent the wheel, in a normal project,
 * there probably would be already an established id scheme
 */
namespace uuid {

	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<> dis(0, 15);
	static std::uniform_int_distribution<> dis2(8, 11);

	std::string generate_uuid_v4() {
		std::stringstream ss;
		int i;
		ss << std::hex;
		for (i = 0; i < 8; i++) {
			ss << dis(gen);
		}
		ss << "-";
		for (i = 0; i < 4; i++) {
			ss << dis(gen);
		}
		ss << "-4";
		for (i = 0; i < 3; i++) {
			ss << dis(gen);
		}
		ss << "-";
		ss << dis2(gen);
		for (i = 0; i < 3; i++) {
			ss << dis(gen);
		}
		ss << "-";
		for (i = 0; i < 12; i++) {
			ss << dis(gen);
		};
		return ss.str();
	}

}

namespace SL {

	enum class LogLevel {
		DEBUG = 0, INFO, WARNING, ERROR
	};

	class LogEntry {
		std::string id;
		LogLevel logLevel;
		std::string message;
		std::string what;

	public:
		LogEntry(LogLevel level, const std::string &message)
				: id(uuid::generate_uuid_v4()), logLevel(level), message(message), what("") {}

		LogEntry(LogLevel level, const std::string &message, const std::string &what)
				: id(uuid::generate_uuid_v4()), logLevel(level), message(message), what(what) {}

		const std::string &getId() const { return id; }

		LogLevel getLevel() const { return logLevel; }

		//maintaining this is a nightmare, but there isn't really a better solution afaik
		std::string getLogLevelAsString() {
			switch (logLevel) {
				case LogLevel::DEBUG:
					return "DEBUG";
				case LogLevel::INFO:
					return "INFO";
				case LogLevel::WARNING:
					return "WARNING";
				case LogLevel::ERROR:
					return "ERROR";
			}

			return "";
		}

		const std::string &getMessage() const { return message; }

		const std::string &getWhat() const { return what; }
	};

	class Logger {
		//console-related
		bool enableConsoleLogging;
		LogLevel minConsoleLogLevel;
		std::string consoleFormat;

		//file-related
		bool enableFileLogging;
		LogLevel minFileLogLevel;
		std::string fileFormat;
		std::string filePrefix;
		uint32_t maxFileLenght;
		uint32_t maxRotation;

	public:
		//usable default values
		Logger() : enableConsoleLogging(false),
				   minConsoleLogLevel(LogLevel::WARNING),
				   consoleFormat(),
				   enableFileLogging(true),
				   minFileLogLevel(LogLevel::WARNING),
				   fileFormat(), filePrefix("log"),
				   maxFileLenght(0),
				   maxRotation(1) {}

		Logger(bool enableConsoleLogging, LogLevel minConsoleLogLevel, const std::string &consoleFormat,
			   bool enableFileLogging, LogLevel minFileLogLevel, const std::string &fileFormat,
			   const std::string &filePrefix, uint32_t maxFileLenght, uint32_t maxRotation)
				: enableConsoleLogging(enableConsoleLogging),
				  minConsoleLogLevel(minConsoleLogLevel),
				  consoleFormat(consoleFormat),
				  enableFileLogging(enableFileLogging),
				  minFileLogLevel(minFileLogLevel),
				  fileFormat(fileFormat),
				  filePrefix(filePrefix),
				  maxFileLenght(maxFileLenght),
				  maxRotation(maxRotation) {}

		void log(LogLevel level, const std::string &message, std::exception *errorClass = nullptr) {
			std::string what = errorClass != nullptr ? errorClass->what() : "";

			LogEntry entry(level, message, what);

			//todo save entry
		}

		std::vector<LogEntry> getErrors() {
			//todo make list of entries, maybe add configurable variable to check for existing ones in files
		}

		void clear(const std::string &id) {
			//todo remove from list
		}
	};

}


#endif //SIMPLELOGGER_HPP