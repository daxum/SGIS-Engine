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

#include "EngineConfig.hpp"

enum LogLevel {
	DEBUG = 1,
	INFO = 2,
	WARN = 4,
	ERROR = 8,
	FATAL = 16,
	SPAM = 32
};

//A simple thread-safe logger.
class Logger {
public:
	/**
	 * Constructs a logger that prints to the given string with the given levels enabled.
	 * @param type The type of output file.
	 * @param mask A set of LogLevel bits indicating which levels to enable. If the bit
	 *     for a level is set, the logger will write messages for that level.
	 * @param filename The filename to write to. Only used for LogType FILE.
	 */
	Logger(const LogType type, uint32_t mask, const std::string& filename = "");

	/**
	 * If necessary, frees the output stream.
	 */
	~Logger();

	/**
	 * The below six functions write messages at their respective logging levels.
	 * If that level's bit is not set, nothing will be written.
	 * @param out The message to write.
	 */
	void debug(const std::string& out);
	void info(const std::string& out);
	void warn(const std::string& out);
	void error(const std::string& out);
	void fatal(const std::string& out);
	void spam(const std::string& out);

private:
	//A pointer to the output stream to log messages to.
	std::ostream* output;
	//Whether to free the output stream when the logger is destructed.
	bool freeOut;
	//The mask of writing bits.
	uint32_t logMask;
};
