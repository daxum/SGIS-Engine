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
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

std::shared_ptr<ModelData> ModelLoader::loadFromDisk(std::string filename) {
	std::shared_ptr<ModelData> data = std::make_shared<ModelData>();

	tinyobj::attrib_t attributes;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string error;

	if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &error, filename.c_str())) {
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
				attributes.normals[3 * index.vertex_index],
				attributes.normals[3 * index.vertex_index + 1],
				attributes.normals[3 * index.vertex_index + 2]
			};

			vertex.texCoords = {
				attributes.texcoords[3 * index.vertex_index],
				attributes.texcoords[3 * index.vertex_index + 1]
			};

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = data->vertices.size();
				data->vertices.push_back(vertex);
			}

			data->indices.push_back(uniqueVertices[vertex]);
		}
	}

	//TODO: Materials

	return data;
}
