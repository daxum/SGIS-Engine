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

#include <fstream>
#include "Logger.hpp"

Logger::Logger(const LogType type, uint32_t mask, const std::string& filename) :
	freeOut(true),
	logMask(mask) {

	switch (type) {
		case LogType::STDOUT: output = &std::cout; freeOut = false; break;
		case LogType::FILE: output = new std::ofstream(filename); break;
		default: throw std::runtime_error("Incomplete switch in Logger!");
	}
}

Logger::~Logger() {
	if (freeOut) {
		delete output;
	}
}

void Logger::debug(const std::string& out) {
	if (logMask & DEBUG) {
		*output << "D::" + out + "\n";
	}
}

void Logger::info(const std::string& out) {
	if (logMask & INFO) {
		*output << "I::" + out + "\n";
	}
}

void Logger::warn(const std::string& out) {
	if (logMask & WARN) {
		*output << "W::" + out + "\n";
	}
}

void Logger::error(const std::string& out) {
	if (logMask & ERROR) {
		*output << "E::" + out + "\n";
	}
}

void Logger::fatal(const std::string& out) {
	if (logMask & FATAL) {
		*output << "F::" + out + "\n";
	}
}
