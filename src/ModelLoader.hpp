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
#include "AxisAlignedBB.hpp"

class ModelManager;

struct ModelData {
	//The vertices in the model's mesh.
	std::vector<Vertex> vertices;
	//The indices for the draw order of the vertices.
	std::vector<uint32_t> indices;
};

struct LightInfo {
	glm::vec3 ka;
	glm::vec3 kd;
	glm::vec3 ks;
	float s;
};

class ModelLoader {
public:
	/**
	 * Constructs a model loader.
	 * @param logConfig The configuration for the logger.
	 * @param modelManager The manager to load the models to.
	 */
	ModelLoader(const LogConfig& logConfig, ModelManager& modelManager) :
		logger(logConfig),
		modelManager(modelManager) {}

	/**
	 * Virtual destructor, does nothing.
	 */
	virtual ~ModelLoader() {}

	/**
	 * Loads a model from disk and makes it ready for use in drawing.
	 * This is a temporary interface until the model loader gets rewritten
	 * to be more flexible. For now, it only supports .obj models.
	 * TODO: Too many parameters. Move to struct?
	 * @param name The name to store the loaded model under.
	 * @param filename The filename for the model to load.
	 * @param texture The texture to use for the model.
	 * @param shader The shader to use for the model.
	 * @param buffer The buffer the model's mesh goes in.
	 * @param uniformSet The uniform set the model uses.
	 * @param lighting The lighting information for the model.
	 * @param viewCull Whether to cull the object when it can't be seen by the camera.
	 */
	void loadModel(const std::string& name, const std::string& filename, const std::string& texture, const std::string& shader, const std::string& buffer, const std::string& uniformSet, const LightInfo& lighting, bool viewCull = true);

protected:
	//The logger.
	Logger logger;
	//Model manager to load models to.
	ModelManager& modelManager;

	/**
	 * Loads a model from disk (currently only .obj is supported).
	 * @param filename The filename of the model to load.
	 * @param vertexBuffer The buffer the mesh goes in, determines vertex format.
	 * @return a pointer to the loaded model data.
	 * @throw runtime_error if model loading failed.
	 */
	std::shared_ptr<ModelData> loadFromDisk(const std::string& filename, const std::string& vertexBuffer);

	/**
	 * Calculates a bounding box for a model's mesh.
	 * @param data The model data to calculate a box for.
	 * @return a box for the mesh.
	 */
	Aabb<float> calculateBox(std::shared_ptr<ModelData> data) const;

	/**
	 * Calculates the maximum radius of the model.
	 * @param data The model data.
	 * @param center The center of the model.
	 * @return The distance of the farthest vertex from the center.
	 */
	float calculateMaxRadius(std::shared_ptr<ModelData> data, glm::vec3 center) const;
};
