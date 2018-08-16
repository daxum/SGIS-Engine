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

#include <iostream>
#include <cstdint>

#define NO_ENGINE_LOG
#include "../src/Logger.hpp"
#include "../src/ExtraMath.hpp"

#ifdef __unix__
	static const LogType logType = LogType::FILE;
	static const std::string file = "/dev/null";
#else
	//Sorry for the spam, windows
	static const LogType logType = LogType::STDOUT;
	static const std::string file = "";
#endif

uint32_t logEnableMask = SPAM | DEBUG | INFO | WARN | ERROR | FATAL;
uint32_t logDisableMask = 0;

int main(int argc, char** argv) {
	Logger logger(logType, logEnableMask, file);
	Logger nullLogger(logType, logDisableMask, file);

	double start = ExMath::getTimeMillis();

	//Should have high overhead
	for (size_t i = 0; i < 1000; i++) {
		logger.spam("test" + std::to_string(i));
	}

	double end = ExMath::getTimeMillis();

	//Should have no overhead (optimized out by compiler)
	//Directly calling logging functions seems to cause overhead from to_string,
	//which isn't constexpr. Macro works fine, though.
	for (size_t i = 0; i < 1000; i++) {
		ENGINE_LOG_SPAM(nullLogger, "test2" + std::to_string(i));
	}

	double endNull = ExMath::getTimeMillis();

	std::cout << "Active logging: " << end - start << ", Null logging: " << endNull - end << "\n";

	return 0;
}
