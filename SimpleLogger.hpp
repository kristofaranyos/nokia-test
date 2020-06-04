#ifndef SIMPLELOGGER_HPP
#define SIMPLELOGGER_HPP


#include <algorithm>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <iomanip>
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

	class InvalidLogIDException : public std::runtime_error {
	public:
		explicit InvalidLogIDException(const std::string &id) : runtime_error(
				"Log with id " + id + " doesn't exist.") {}
	};

	class LogEntry {
		std::string id;
		LogLevel logLevel;
		time_t time;
		std::string message;
		std::string what;

	public:
		LogEntry(LogLevel level, std::chrono::time_point<std::chrono::system_clock> time, const std::string &message)
				: id(uuid::generate_uuid_v4()), logLevel(level), time(std::chrono::system_clock::to_time_t(time)),
				  message(message), what("") {}

		LogEntry(LogLevel level, std::chrono::time_point<std::chrono::system_clock> time, const std::string &message,
				 const std::string &what)
				: id(uuid::generate_uuid_v4()), logLevel(level), time(std::chrono::system_clock::to_time_t(time)),
				  message(message), what(what) {}

		const std::string &getId() const { return id; }

		LogLevel getLevel() const { return logLevel; }

		//maintaining this is a nightmare, but there isn't really a better solution afaik
		std::string getLogLevelAsString() const {
			switch (logLevel) {
				case LogLevel::DEBUG:
					return "DEBUG..";
				case LogLevel::INFO:
					return "INFO...";
				case LogLevel::WARNING:
					return "WARNING";
				case LogLevel::ERROR:
					return "ERROR..";
			}
		}

		const time_t &getTime() const { return time; }

		const std::string &getMessage() const { return message; }

		const std::string &getWhat() const { return what; }
	};

	class Logger {
		//console-related
		bool enableConsoleLogging;
		LogLevel minConsoleLogLevel;
		std::string consoleFormat;
		std::ostream *consoleStream;

		//file-related
		bool enableFileLogging;
		LogLevel minFileLogLevel;
		std::string fileFormat;
		std::string filePrefix;
		uint32_t maxFileLenght;
		uint32_t maxRotation;

		std::string dateFormat;

		std::vector<LogEntry> entries;

		//uses put_time to format the time
		std::string getFormattedTime(time_t time) const {
			std::stringstream ss;
			ss << std::put_time(std::localtime(&time), dateFormat.c_str());
			return ss.str();
		}

		/*
		 * Format settings: you can write whatever you want (lowercase, though)
		 * and the function will substitute the data in place of the tokens
		 *
		 * Tokens:
		 * - I : id
		 * - T : time
		 * - L : LogLevel
		 * - M : Message
		 * - W : Output of what() if any
		 *
		 * Example: "[T] L, M, W"
		 */
		std::string getFormattedEntry(const LogEntry &entry, const std::string &lineFormat) const {
			std::string formattedEntry;

			std::for_each(lineFormat.begin(), lineFormat.end(), [&](char c) {
				std::string substitute;

				if (c == 'I') { substitute = entry.getId(); }
				if (c == 'T') { substitute = getFormattedTime(entry.getTime()); }
				if (c == 'L') { substitute = entry.getLogLevelAsString(); }
				if (c == 'M') { substitute = entry.getMessage(); }
				if (c == 'W') {
					if (entry.getWhat().empty()) { substitute = "[empty]"; }
					else { substitute = entry.getWhat(); }

				}

				formattedEntry += substitute.empty() ? std::string(1, c) : substitute;
			});

			return formattedEntry;
		}

	public:
		//usable default values
		Logger() : enableConsoleLogging(false),
				   minConsoleLogLevel(LogLevel::WARNING),
				   consoleFormat("[T] L, M, W"),
				   consoleStream(&std::cout), //could be cerr too
				   enableFileLogging(true),
				   minFileLogLevel(LogLevel::WARNING),
				   fileFormat("[T] L, M, W"), filePrefix("log"),
				   maxFileLenght(0),
				   maxRotation(1),
				   dateFormat("%Y-%m-%d %X") {}

		//fine tune constructor, prefer to use basic one and just set whatever you need later
		Logger(bool enableConsoleLogging, LogLevel minConsoleLogLevel, const std::string &consoleFormat,
			   std::ostream &consoleStream, bool enableFileLogging, LogLevel minFileLogLevel,
			   const std::string &fileFormat, const std::string &filePrefix, uint32_t maxFileLenght,
			   uint32_t maxRotation, const std::string &dateFormat)
				: enableConsoleLogging(enableConsoleLogging),
				  minConsoleLogLevel(minConsoleLogLevel),
				  consoleFormat(consoleFormat),
				  consoleStream(&consoleStream),
				  enableFileLogging(enableFileLogging),
				  minFileLogLevel(minFileLogLevel),
				  fileFormat(fileFormat),
				  filePrefix(filePrefix),
				  maxFileLenght(maxFileLenght),
				  maxRotation(maxRotation),
				  dateFormat(dateFormat) {}

		/*
		 * The function always saves the logs in it's temporary store, but only outputs to console or file when the
		 * log level is above threshold
		 */
		void log(LogLevel level, const std::string &message, std::exception *errorClass = nullptr) {
			std::string what = errorClass != nullptr ? errorClass->what() : "";

			LogEntry entry(level, std::chrono::system_clock::now(), message, what);
			entries.emplace_back(entry);

			if (enableConsoleLogging && level >= minConsoleLogLevel) {
				*consoleStream << getFormattedEntry(entry, consoleFormat) << std::endl;
			}

			if (enableFileLogging && level >= minFileLogLevel) {
				//todo
			}
		}

		const std::vector<LogEntry> &getErrors() const {
			return entries;
		}

		void clear(const std::string &id) {
			auto it = std::find_if(entries.begin(), entries.end(), [&id](const LogEntry &entry) { return entry.getId() == id; });

			if (it == entries.end()) {
				this->log(LogLevel::ERROR, "Couldn't delete entry with id " + id + " from log.");
				throw InvalidLogIDException(id);
			}

			entries.erase(it);
		}

		//setters return reference to current object to work as fluent interface

		Logger &setConsoleLogging(bool b) {
			enableConsoleLogging = b;
			return *this;
		}

		Logger &setMinConsoleLogLevel(LogLevel l) {
			minConsoleLogLevel = l;
			return *this;
		}

		Logger &setConsoleFormat(const std::string &f) {
			consoleFormat = f;
			return *this;
		}

		Logger &setConsoleStream(std::ostream &s) {
			consoleStream = &s;
			return *this;
		}

		Logger &setFileLogging(bool b) {
			enableFileLogging = b;
			return *this;
		}

		Logger &setMinFileLevel(LogLevel l) {
			minFileLogLevel = l;
			return *this;
		}

		Logger &setFileFormat(const std::string &f) {
			fileFormat = f;
			return *this;
		}

		Logger &setFilePrefix(const std::string &p) {
			filePrefix = p;
			return *this;
		}

		Logger &setmaxFileLength(uint32_t l) {
			maxFileLenght = l;
			return *this;
		}

		Logger &setMaxRotation(uint32_t r) {
			maxRotation = r;
			return *this;
		}

		Logger &setDateFormat(const std::string &f) {
			dateFormat = f;
			return *this;
		}
	};

}


#endif //SIMPLELOGGER_HPP