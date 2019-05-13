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
#include <vector>

#include "Mesh.hpp"

class MeshBuilder {
public:
	/**
	 * Initializes a mesh builder with the given format and enough space for
	 * the provided number of vertices.
	 * @param format The format of the mesh's vertices.
	 * @param maxVerts The maximum number of vertices in the mesh.
	 */
	MeshBuilder(const VertexFormat* format, size_t maxVerts);

	/**
	 * Mercilessly destroys the vertex data.
	 */
	~MeshBuilder() { delete[] vertexData; }

	/**
	 * Can't have these screwing up the pointers.
	 */
	MeshBuilder(const MeshBuilder&) = delete;
	MeshBuilder(MeshBuilder&&) = delete;

	/**
	 * Adds a vertex to the resulting mesh, assigning it the next index
	 * if it is not a duplicate.
	 * @param vert The vertex to add.
	 */
	void addVertex(const Vertex& vert);

	/**
	 * Generates the mesh using all added vertices.
	 * @param bufferInfo The buffer info for the mesh.
	 * @return The generated mesh.
	 */
	Mesh genMesh(Mesh::BufferInfo bufferInfo);

	/**
	 * Gets the number of vertices in the mesh.
	 * @return The number of vertices.
	 */
	size_t vertexCount() const { return numVerts; }

	/**
	 * Gets the current number of indices in the mesh.
	 * @return The number of indices.
	 */
	size_t indexCount() const { return indices.size(); }

private:
	//Format of the stored vertices.
	const VertexFormat* format;
	//Indexes of all the vertices added so far.
	std::unordered_map<Vertex, uint32_t> uniqueVertices;
	//List of vertex indices.
	std::vector<uint32_t> indices;
	//Current number of added vertices.
	size_t numVerts;
	//Vertex data store, to be directly copied to the mesh.
	unsigned char* vertexData;
	//The maximum allowed number of vertices in the mesh.
	size_t maxVerts;
	//Whether the format has a position element suitable for calculating the radius/box.
	bool hasPos;
	//Current box of the mesh.
	Aabb<float> meshBox;
	//Square of the current radius of the mesh.
	float radiusSq;
};
