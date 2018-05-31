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
#include <unordered_map>

#include "Vertex.hpp"
#include "Logger.hpp"
#include "Model.hpp"
#include "AxisAlignedBB.hpp"

class RenderingEngine;

struct ModelData {
	//The vertices in the model's mesh
	std::vector<Vertex> vertices;
	//The indices for the draw order of the vertices
	std::vector<uint32_t> indices;
};

class ModelLoader {
public:
	/**
	 * Constructs a model loader.
	 * @param renderer The rendering engine to upload models to.
	 * @param modelMap The map to store loaded models in.
	 * @param logger The logger to use.
	 */
	ModelLoader(std::shared_ptr<RenderingEngine>& renderer, std::unordered_map<std::string, Model>& modelMap, Logger& logger) : logger(logger), renderer(renderer), models(modelMap) {}

	/**
	 * Virtual destructor, does nothing.
	 */
	virtual ~ModelLoader() {}

	/**
	 * Loads a model from disk and makes it ready for use in drawing.
	 * @param name The name to store the loaded model under.
	 * @param filename The filename for the model to load.
	 * @param texture The texture to use for the model.
	 * @param shader The shader to use for the model.
	 * @param lighting The lighting information for the model.
	 * @param pass The render pass to render the model in.
	 */
	void loadModel(std::string name, std::string filename, std::string texture, std::string shader, LightInfo lighting, RenderPass pass);

protected:
	//The logger
	Logger& logger;

	//The rendering engine used. Keeping a reference is rather silly and
	//mostly undesired, but it solves the initialization order problem in Engine.
	//Hopefully can find a better way later.
	std::shared_ptr<RenderingEngine>& renderer;

	//A map to load models to.
	std::unordered_map<std::string, Model>& models;

	/**
	 * Loads a model from disk (currently only .obj is supported).
	 * @param filename The filename of the model to load.
	 * @return a pointer to the loaded model data.
	 * @throw runtime_error if model loading failed.
	 */
	std::shared_ptr<ModelData> loadFromDisk(std::string filename);

	/**
	 * Calculates a bounding box for a model's mesh.
	 * @param data The model data to calculate a box for.
	 * @return a box for the mesh.
	 */
	AxisAlignedBB calculateBox(std::shared_ptr<ModelData> data);

	/**
	 * Calculates the maximum radius of the model.
	 * @param data The model data.
	 * @param center The center of the model.
	 * @return The distance of the farthest vertex from the center.
	 */
	float calculateMaxRadius(std::shared_ptr<ModelData> data, glm::vec3 center);
};
