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

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

#include "Vertex.hpp"
#include "Logger.hpp"

struct ModelData {
	//The vertices in the model's mesh
	std::vector<Vertex> vertices;
	//The indices for the draw order of the vertices
	std::vector<uint32_t> indices;
};

class ModelLoader {
public:
	/**
	 * Constructs a model loader. Sets the logger.
	 */
	ModelLoader(Logger& logger) : logger(logger) {}

	/**
	 * Virtual destructor, does nothing.
	 */
	virtual ~ModelLoader() {}

	/**
	 * Loads a model from disk and makes it ready for use in drawing.
	 * @param name The name to store the loaded model under.
	 * @param filename The filename for the model to load.
	 */
	virtual void loadModel(std::string name, std::string filename, std::string texture) = 0;

protected:
	//The logger
	Logger& logger;
	/**
	 * Loads a model from disk (currently only .obj is supported).
	 * @param filename The filename of the model to load.
	 * @return a pointer to the loaded model data.
	 * @throw runtime_error if model loading failed.
	 */
	std::shared_ptr<ModelData> loadFromDisk(std::string filename);
};
