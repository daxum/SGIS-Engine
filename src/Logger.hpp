/******************************************************************************
 * SGIS-Engine - the engine for SGIS
 * Copyright (C) 2018
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#pragma once

#include <iostream>
#include <cstdint>
#include <memory>
#include <string>
#include <fstream>

//Used to completely disable all engine logging.
//This seems necessary because the string parameters
//need to be concatenated even when logging is disabled,
//and the if statements aren't enough for the compiler to
//optimize out the statements.
//First parameter is the logger object, second is the message.
//TODO: Global enable / disable for each level?
#ifdef NO_ENGINE_LOG
#	define ENGINE_LOG_SPAM(x, y)
#	define ENGINE_LOG_DEBUG(x, y)
#	define ENGINE_LOG_INFO(x, y)
#	define ENGINE_LOG_WARN(x, y)
#	define ENGINE_LOG_ERROR(x, y)
#	define ENGINE_LOG_FATAL(x, y)
#else
#	define ENGINE_LOG_SPAM(x, y) x.spam(y)
#	define ENGINE_LOG_DEBUG(x, y) x.debug(y)
#	define ENGINE_LOG_INFO(x, y) x.info(y)
#	define ENGINE_LOG_WARN(x, y) x.warn(y)
#	define ENGINE_LOG_ERROR(x, y) x.error(y)
#	define ENGINE_LOG_FATAL(x, y) x.fatal(y)
#endif

enum LogLevel {
	DEBUG = 1,
	INFO = 2,
	WARN = 4,
	ERROR = 8,
	FATAL = 16,
	SPAM = 32
};

//Type of logger output.
enum class LogType {
	STDOUT,
	FILE
};

//Specifies logging information.
struct LogConfig {
	//The type of output for the logger.
	LogType type;
	//Only used for LogType FILE
	std::string outputFile;
	//The level mask - can be {DEBUG|INFO|WARN|ERROR|FATAL}
	//If a bit is unset, that level will be disabled.
	uint32_t mask;
};

//A simple thread-safe logger.
class Logger {
public:
	/**
	 * Constructs a logger that prints to the given string with the given levels enabled.
	 * @param config The configuration for this logger.
	 */
	Logger(const LogConfig& config) :
		freeOut(true),
		logMask(config.mask) {

		switch (config.type) {
			case LogType::STDOUT: output = &std::cout; freeOut = false; break;
			case LogType::FILE: output = new std::ofstream(config.outputFile); break;
			default: throw std::runtime_error("Incomplete switch in Logger!");
		}
	}

	/**
	 * If necessary, frees the output stream.
	 */
	~Logger() {
		if (freeOut) {
			delete output;
		}
	}

	/**
	 * The below six functions write messages at their respective logging levels.
	 * If that level's bit is not set, nothing will be written.
	 * @param out The message to write.
	 */
	void debug(const std::string& out) const {
		if (logMask & DEBUG) {
			*output << "D::" + out + "\n";
		}
	}

	void info(const std::string& out) const {
		if (logMask & INFO) {
			*output << "I::" + out + "\n";
		}
	}

	void warn(const std::string& out) const {
		if (logMask & WARN) {
			*output << "W::" + out + "\n";
		}
	}

	void error(const std::string& out) const {
		if (logMask & ERROR) {
			*output << "E::" + out + "\n";
		}
	}

	void fatal(const std::string& out) const {
		if (logMask & FATAL) {
			*output << "F::" + out + "\n";
		}
	}

	void spam(const std::string& out) const {
		if (logMask & SPAM) {
			*output << "S::" + out + "\n";
		}
	}

private:
	//A pointer to the output stream to log messages to.
	std::ostream* output;
	//Whether to free the output stream when the logger is destructed.
	bool freeOut;
	//The mask of writing bits.
	uint32_t logMask;
};
