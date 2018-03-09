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

#include "Model.hpp"
#include "ModelLoader.hpp"

class ModelManager {
public:
	/**
	 * Creates a model manager.
	 * @param logConfig The configuration for the manager's logger.
	 * @param renderer The rendering engine, used for model loading.
	 */
	ModelManager(const LogConfig& logConfig, std::shared_ptr<RenderingEngine>& renderer) :
		logger(logConfig.type, logConfig.mask, logConfig.outputFile),
		modelMap(),
		loader(renderer, modelMap, logger) {}

	/**
	 * Gets the model loader for this manager.
	 * @return The model loader.
	 */
	ModelLoader& getLoader() { return loader; }

	/**
	 * Retreives the model with the given name.
	 * @param name The name of the model to get.
	 * @return the model with the given name.
	 * @throw Whatever maps throw when they don't have a value (runtime_error?)
	 */
	Model& getModel(std::string name) { return modelMap.at(name); }

private:
	//The logger
	Logger logger;

	//A map to store loaded model information
	std::unordered_map<std::string, Model> modelMap;

	//The model loader
	ModelLoader loader;
};
