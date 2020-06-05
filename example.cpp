#include <iostream>

#include "SimpleLogger.hpp"


class RandomErrorClass : public std::exception {
	const char *what() const noexcept override {
		return "you forgot how to quit vim";
	}
};

using namespace SL;

int main() {
	Logger logger1;
	//you can chain settings directly, these settings enable console logging for easy debugging
	logger1.setConsoleLogging(true).setConsoleFormat("[T] L, M, W").setMinConsoleLogLevel(LogLevel::DEBUG);

	logger1.log(LogLevel::ERROR, "test1");
	logger1.log(LogLevel::WARNING, "test2");
	logger1.log(LogLevel::INFO, "test3", new RandomErrorClass);

	//clear the second log
	logger1.clear(logger1.getErrors()[1].getId());



	Logger logger2;
	//setting custom name, amount of log files and file length (10*10=100 entries are saved at most)
	logger2.setFileName("serverLog[n].log").setMaxRotation(10).setmaxFileLength(10);

	//log some more..
}