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

#include "AxisAlignedBB.hpp"
#include "Vertex.hpp"
#include "VertexFormat.hpp"

//Caching level for meshes.
enum CacheLevel {
	DISK,
	MEMORY,
	GPU,
	NUM_LEVELS
};

class Mesh {
public:
	/**
	 * Creates a mesh with the given vertices and indices.
	 * @param buffer The buffer to place the mesh in for rendering.
	 * @param format The format for the mesh's vertex data.
	 * @param vertices The vertices for the mesh. Their internal data is copied into the mesh.
	 * @param indices The index data for the mesh.
	 * TODO: calculate two below in constructor.
	 * @param box The bounding box for the mesh.
	 * @param radius The radius of the mesh.
	 */
	Mesh(const std::string& buffer, const VertexFormat* format, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const Aabb<float>& box, float radius);

	/**
	 * Copy constructor.
	 */
	Mesh(const Mesh& mesh);

	/**
	 * Move constructor.
	 */
	Mesh(Mesh&& mesh);

	/**
	 * Destructor.
	 */
	~Mesh() { delete[] vertexData; }

	/**
	 * Gets the buffer the mesh is stored in.
	 * @return the vertex buffer this mesh belongs to.
	 */
	const std::string& getBuffer() const { return buffer; }

	/**
	 * Gets the format that the mesh's vertex data is in.
	 * @return The mesh's vertex format.
	 */
	const std::vector<VertexElement>& getFormat() const { return format; }

	/**
	 * Gets the mesh's bounding box.
	 * @return the bounding box of the mesh.
	 */
	const Aabb<float>& getBox() const { return box; }

	/**
	 * Get's the mesh's radius.
	 * @return The radius of the mesh.
	 */
	float getRadius() const { return radius; }

	/**
	 * Retrieves all the mesh data, for uploading into a vertex / index buffer.
	 */
	const std::tuple<const unsigned char*, size_t, const std::vector<uint32_t>&> getMeshData() const {
		return {vertexData, vertexSize, indices};
	}

	/**
	 * Copy assignment.
	 */
	Mesh& operator=(const Mesh& mesh) {
		if (this != &mesh) {
			if (vertexSize != mesh.vertexSize) {
				delete[] vertexData;
				vertexSize = 0;
				vertexData = nullptr;
				vertexData  = new unsigned char[mesh.vertexSize];
				vertexSize = mesh.vertexSize;
			}

			memcpy(vertexData, mesh.vertexData, vertexSize);
			indices = mesh.indices;
			buffer = mesh.buffer;
			format = mesh.format;
			box = mesh.box;
			radius = mesh.radius;
		}

		return *this;
	}

	/**
	 * Move assignment.
	 */
	Mesh& operator=(Mesh&& mesh) {
		if (this != &mesh) {
			delete[] vertexData;

			vertexData = std::exchange(mesh.vertexData, nullptr);
			vertexSize = std::exchange(mesh.vertexSize, 0);
			indices = std::move(mesh.indices);
			buffer = std::move(mesh.buffer);
			format = std::exchange(mesh.format, nullptr);
			box = std::move(mesh.box);
			radius = std::exchange(mesh.radius, 0.0f);
		}

		return *this;
	}

private:
	//The mesh's vertex data and size.
	unsigned char* vertexData;
	size_t vertexSize;

	//Indices for the vertices.
	std::vector<uint32_t> indices;

	//The buffer this mesh belongs in.
	std::string buffer;

	//The format the mesh's vertex data is in.
	const VertexFormat* format;

	//Possibly useful dimensions for the mesh, calculated on construction.
	Aabb<float> box;
	float radius;
};
