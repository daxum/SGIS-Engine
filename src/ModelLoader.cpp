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

#include <stdexcept>
#include <unordered_map>
#include "ModelLoader.hpp"
#include "RenderingEngine.hpp"
#include "Engine.hpp"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

void ModelLoader::loadModel(std::string name, std::string filename, std::string texture, std::string shader, LightInfo lighting, RenderPass pass, bool viewCull) {
	std::shared_ptr<ModelData> data = loadFromDisk(filename);
	//Always static for now
	std::shared_ptr<RenderMeshObject> mesh = renderer->getMemoryManager()->addStaticMesh(data->vertices, data->indices);

	AxisAlignedBB box = calculateBox(data);
	logger.debug("Calculated box " + box.toString() + " for model " + name);

	float radius = calculateMaxRadius(data, box.getCenter());
	logger.debug("Radius of model is " + std::to_string(radius));

	models.insert(std::make_pair(name, Model(mesh, box, radius, texture, shader, lighting, pass, viewCull)));
	logger.debug("Loaded model \"" + filename + "\" as \"" + name + "\".");
}

std::shared_ptr<ModelData> ModelLoader::loadFromDisk(std::string filename) {
	std::shared_ptr<ModelData> data = std::make_shared<ModelData>();

	logger.debug("Loading model \"" + filename + "\".");

	tinyobj::attrib_t attributes;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string error;

	if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &error, filename.c_str())) {
		logger.fatal("Failed to load model \"" + filename + "\"!");
		throw std::runtime_error(error);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices;

	for (const tinyobj::shape_t& shape : shapes) {
		for (const tinyobj::index_t& index : shape.mesh.indices) {
			Vertex vertex = {};

			vertex.pos = {
				attributes.vertices[3 * index.vertex_index],
				attributes.vertices[3 * index.vertex_index + 1],
				attributes.vertices[3 * index.vertex_index + 2]
			};

			vertex.normal = {
				attributes.normals[3 * index.normal_index],
				attributes.normals[3 * index.normal_index + 1],
				attributes.normals[3 * index.normal_index + 2]
			};

			if (!attributes.texcoords.empty()) {
				vertex.texCoords = {
					attributes.texcoords[2 * index.texcoord_index],
					attributes.texcoords[2 * index.texcoord_index + 1]
				};
			}
			else {
				vertex.texCoords = {0.0, 0.0};
			}

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = data->vertices.size();
				data->vertices.push_back(vertex);
			}

			data->indices.push_back(uniqueVertices[vertex]);
		}
	}

	logger.debug("File \"" + filename + "\" loaded from disk. Stats:" +
				 "\n\tVertices:          " + std::to_string(data->vertices.size()) +
				 "\n\tIndices:           " + std::to_string(data->indices.size()) +
				 "\n\tTotal loaded size: " + std::to_string(data->vertices.size() * sizeof(Vertex) + data->indices.size() * sizeof(uint32_t)) + " bytes");

	return data;
}

AxisAlignedBB ModelLoader::calculateBox(std::shared_ptr<ModelData> data) {
	if (data->vertices.size() == 0) {
		logger.warn("Zero vertex mesh loaded?!");
		return AxisAlignedBB(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 0.0));
	}

	glm::vec3 max(data->vertices[0].pos.x, data->vertices[0].pos.y, data->vertices[0].pos.z);
	glm::vec3 min(max);

	for (size_t i = 1; i < data->vertices.size(); i++) {
		const glm::vec3& current = data->vertices[i].pos;

		max.x = std::max(max.x, current.x);
		max.y = std::max(max.y, current.y);
		max.z = std::max(max.z, current.z);

		min.x = std::min(min.x, current.x);
		min.y = std::min(min.y, current.y);
		min.z = std::min(min.z, current.z);
	}

	return AxisAlignedBB(min, max);
}

float ModelLoader::calculateMaxRadius(std::shared_ptr<ModelData> data, glm::vec3 center) {
	if (data->vertices.size() == 0) {
		logger.warn("Zero vertex mesh loaded?!");
		return 0.0f;
	}

	float maxDistSq = 0.0f;

	for (size_t i = 0; i < data->vertices.size(); i++) {
		const glm::vec3& current = data->vertices[i].pos;

		float vertDistSq = glm::dot(current, current);

		if (maxDistSq < vertDistSq) {
			maxDistSq = vertDistSq;
		}
	}

	return sqrt(maxDistSq);
}
