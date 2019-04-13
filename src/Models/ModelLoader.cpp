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
#include "ModelManager.hpp"
#include "Engine.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

void ModelLoader::loadModel(const std::string& name, const std::string& filename, const std::string& texture, const std::string& shader, const std::string& bufferName, const std::string& uniformSet, bool viewCull) {
	if (!modelManager.hasMesh(filename)) {
		const VertexFormat* format = nullptr; //TODO: Get from wherever vertex formats are stored
		loadMesh(filename, format, bufferName);
	}

	Material material(name, shader, uniformSet, modelManager.getMemoryManager()->getUniformSet(uniformSet), viewCull);
	material.textures.push_back(texture);

	if (material.uniforms.hasUniform("ka", UniformType::VEC3)) {
		material.uniforms.setVec3("ka", {1.0f, 0.0f, 1.0f});
	}

	if (material.uniforms.hasUniform("kd", UniformType::VEC3)) {
		material.uniforms.setVec3("kd", {1.0f, 0.0f, 1.0f});
	}

	if (material.uniforms.hasUniform("ks", UniformType::VEC3)) {
		material.uniforms.setVec3("ks", {1.0f, 0.0f, 1.0f});
	}

	if (material.uniforms.hasUniform("s", UniformType::FLOAT)) {
		material.uniforms.setFloat("s", 1.0f);
	}

	modelManager.addMaterial(name, std::move(material));
	ENGINE_LOG_DEBUG(logger, "Loaded model \"" + Engine::instance->getConfig().resourceBase + filename + "\" as \"" + name + "\".");
}

void ModelLoader::loadMesh(const std::string& filename, const VertexFormat* format, const std::string& bufferName) {
	std::shared_ptr<ModelData> data = loadFromDisk(Engine::instance->getConfig().resourceBase + filename, format);

	Aabb<float> box = calculateBox(data);
	ENGINE_LOG_DEBUG(logger, "Calculated box " + box.toString() + " for mesh " + filename);

	float radius = calculateMaxRadius(data, box.getCenter());
	ENGINE_LOG_DEBUG(logger, "Radius of mesh is " + std::to_string(radius));

	Mesh::BufferInfo bufferInfo = {};
	bufferInfo.vertexName = bufferName;
	bufferInfo.indexName = bufferName + "idx";
	bufferInfo.vertex = modelManager.getMemoryManager()->getBuffer(bufferInfo.vertexName);
	bufferInfo.index = modelManager.getMemoryManager()->getBuffer(bufferInfo.indexName);

	modelManager.addMesh(filename, Mesh(bufferInfo, format, data->vertices, data->indices, box, radius), true);
}

std::shared_ptr<ModelData> ModelLoader::loadFromDisk(const std::string& filename, const VertexFormat* format) {
	std::shared_ptr<ModelData> data = std::make_shared<ModelData>();

	ENGINE_LOG_DEBUG(logger, "Loading model \"" + filename + "\".");

	tinyobj::attrib_t attributes;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn;
	std::string error;

	if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &warn, &error, filename.c_str())) {
		ENGINE_LOG_FATAL(logger, "Failed to load model \"" + filename + "\"!");
		throw std::runtime_error(error);
	}

	if (!warn.empty()) {
		ENGINE_LOG_WARN(logger, warn);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices;

	for (const tinyobj::shape_t& shape : shapes) {
		for (const tinyobj::index_t& index : shape.mesh.indices) {
			Vertex vertex(format);

			if (format->hasElement(VERTEX_ELEMENT_POSITION)) {
				vertex.setVec3(VERTEX_ELEMENT_POSITION, glm::vec3(
					attributes.vertices[3 * index.vertex_index],
					attributes.vertices[3 * index.vertex_index + 1],
					attributes.vertices[3 * index.vertex_index + 2]
				));
			}

			if (format->hasElement(VERTEX_ELEMENT_NORMAL)) {
				vertex.setVec3(VERTEX_ELEMENT_NORMAL, glm::vec3(
					attributes.normals[3 * index.normal_index],
					attributes.normals[3 * index.normal_index + 1],
					attributes.normals[3 * index.normal_index + 2]
				));
			}

			if (format->hasElement(VERTEX_ELEMENT_TEXTURE)) {
				if (!attributes.texcoords.empty()) {
					vertex.setVec2(VERTEX_ELEMENT_TEXTURE, glm::vec2(
						attributes.texcoords[2 * index.texcoord_index],
						attributes.texcoords[2 * index.texcoord_index + 1]
					));
				}
				else {
					vertex.setVec2(VERTEX_ELEMENT_TEXTURE, glm::vec2(0.0, 0.0));
				}
			}

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = data->vertices.size();
				data->vertices.push_back(vertex);
			}

			data->indices.push_back(uniqueVertices.at(vertex));
		}
	}

	ENGINE_LOG_DEBUG(logger,
		"File \"" + filename + "\" loaded from disk. Stats:" +
		"\n\tVertices:          " + std::to_string(data->vertices.size()) +
		"\n\tIndices:           " + std::to_string(data->indices.size()) +
		"\n\tTotal loaded size: " + std::to_string(data->vertices.size() * (data->vertices.empty() ? 0 : data->vertices.at(0).getSize()) + data->indices.size() * sizeof(uint32_t)) + " bytes");

	return data;
}

Aabb<float> ModelLoader::calculateBox(std::shared_ptr<ModelData> data) const {
	if (data->vertices.size() == 0) {
		ENGINE_LOG_WARN(logger, "Zero vertex mesh loaded?!");
		return Aabb<float>(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 0.0));
	}

	glm::vec3 max = data->vertices.at(0).getVec3(VERTEX_ELEMENT_POSITION);
	glm::vec3 min(max);

	for (size_t i = 1; i < data->vertices.size(); i++) {
		const glm::vec3 current = data->vertices.at(i).getVec3(VERTEX_ELEMENT_POSITION);

		max.x = std::max(max.x, current.x);
		max.y = std::max(max.y, current.y);
		max.z = std::max(max.z, current.z);

		min.x = std::min(min.x, current.x);
		min.y = std::min(min.y, current.y);
		min.z = std::min(min.z, current.z);
	}

	return Aabb<float>(min, max);
}

float ModelLoader::calculateMaxRadius(std::shared_ptr<ModelData> data, glm::vec3 center) const {
	if (data->vertices.size() == 0) {
		ENGINE_LOG_WARN(logger, "Zero vertex mesh loaded?!");
		return 0.0f;
	}

	float maxDistSq = 0.0f;

	for (size_t i = 0; i < data->vertices.size(); i++) {
		const glm::vec3 current = data->vertices.at(i).getVec3(VERTEX_ELEMENT_POSITION);

		float vertDistSq = glm::dot(current, current);

		if (maxDistSq < vertDistSq) {
			maxDistSq = vertDistSq;
		}
	}

	return sqrt(maxDistSq);
}