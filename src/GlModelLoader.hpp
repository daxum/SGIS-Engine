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

#include <unordered_map>
#include <string>

#include "ModelLoader.hpp"
#include "GlMemoryManager.hpp"

class GlModelLoader : public ModelLoader {
public:
	/**
	 * Initializes the model loader.
	 * @param modelMap The location where references to loaded models will be stored.
	 * @param memoryManager The place where model data will be sent to be uploaded to the gpu.
	 */
	GlModelLoader(Logger& logger, std::unordered_map<std::string, Model>& modelMap, GlMemoryManager& memoryManager);

	/**
	 * Loads a model from disk to the gpu.
	 * @param name The name the model will be stored under.
	 * @param filename The filename to load the model from.
	 * @throw runtime_error if model loading failed.
	 */
	void loadModel(std::string name, std::string filename);

private:
	//The map to store loaded model data in
	std::unordered_map<std::string, Model>& models;
	//The memory manager to upload model data to.
	GlMemoryManager& memoryManager;
};
