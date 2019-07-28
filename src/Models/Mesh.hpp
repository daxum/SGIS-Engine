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
#include <cstdint>

#include "AxisAlignedBB.hpp"
#include "Vertex.hpp"
#include "VertexFormat.hpp"
#include "Renderer/Buffer.hpp"

class ModelManager;

//Caching level for meshes.
enum CacheLevel {
	DISK,
	MEMORY,
	GPU,
	NUM_LEVELS
};

class Mesh {
public:
	struct BufferInfo {
		const Buffer* vertex;
		const Buffer* index;
		std::string vertexName;
		std::string indexName;
	};

	/**
	 * Creates a mesh with the given vertices and indices.
	 * @param bufferInfo The info about the buffers the mesh is stored in.
	 * @param format The format for the mesh's vertex data.
	 * @param vertices The vertex data for the mesh.
	 * @param indices The index data for the mesh.
	 * @param box The bounding box for the mesh.
	 * @param radius The radius of the mesh.
	 */
	Mesh(BufferInfo bufferInfo, const VertexFormat* format, std::vector<unsigned char>&& vertices, std::vector<uint32_t>&& indices, const Aabb<float>& box, float radius);

	/**
	 * Gets the vertex and index buffers the mesh is stored in.
	 * @return the vertex buffer and index buffers this mesh belongs in.
	 *     If the mesh is not for rendering, the buffer pointers will be
	 *     null and the name strings will be empty.
	 */
	const BufferInfo& getBufferInfo() const { return bufferInfo; }

	/**
	 * Gets the format that the mesh's vertex data is in.
	 * @return The mesh's vertex format.
	 */
	const VertexFormat* getFormat() const { return format; }

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
		return {vertexData.data(), vertexData.size(), indices};
	}

	/**
	 * Returns whether this mesh can be rendered.
	 * @return Whether this mesh is usable by the rendering engine.
	 */
	bool isForRendering() { return bufferInfo.vertex != nullptr; }

	/**
	 * Sets the offset into the index buffer for the mesh. Should only be called from
	 * the renderer memory manager, unless you want to break things.
	 * @param newStart The offset into the index buffer of the mesh's first index.
	 */
	void setIndexOffset(uintptr_t newStart) { indexStart = newStart; }

	/**
	 * Sets the value to be added to each index when accessing the vertex buffer.
	 * This value should also only be set by the renderer memory manager.
	 * @param newOffset The offset of the mesh's first vertex in the vertex buffer.
	 */
	void setVertexOffset(int32_t newOffset) { vertexOffset = newOffset; }

	/**
	 * Gets the data needed to render the mesh. The tuple contains the index offset in
	 * the first value, the index count in the second, and the vertex offset in the third.
	 * @return The index info needed to render the mesh.
	 */
	const std::tuple<uintptr_t, uint32_t, int32_t> getRenderInfo() const { return {indexStart, indices.size(), vertexOffset}; }

private:
	//The mesh's vertex data and size.
	std::vector<unsigned char> vertexData;
	//Indices for the vertices.
	std::vector<uint32_t> indices;
	//Information about the vertex and index buffers this mesh belongs in.
	BufferInfo bufferInfo;
	//The format the mesh's vertex data is in.
	const VertexFormat* format;
	//Possibly useful dimensions for the mesh, calculated on construction.
	Aabb<float> box;
	float radius;
	//Offset into the index buffer of the mesh's index data.
	uintptr_t indexStart;
	//Offset into the mesh's vertex buffer the first index should point to.
	//This is a number which gets added to all indices when rendering.
	int32_t vertexOffset;
};

class MeshRef {
public:
	/**
	 * Creates a reference to the given mesh.
	 * @param manager The model manager that created this reference.
	 * @param meshName The name of the referenced mesh.
	 * @param mesh The mesh to reference.
	 * @param level The required cache level for the mesh.
	 */
	MeshRef(ModelManager* manager, const std::string& meshName, Mesh* mesh, CacheLevel level) :
		manager(manager),
		mesh(mesh),
		meshName(meshName),
		level(level) {}

	/**
	 * Decrements the mesh's reference count.
	 */
	~MeshRef();

	/**
	 * Returns the mesh this reference is referencing. The returned
	 * pointer is only guaranteed to have the same lifespan as the
	 * reference object it was retrieved from.
	 */
	const Mesh* getMesh() const { return mesh; }

	/**
	 * Returns the name of the mesh.
	 * @return The mesh's name.
	 */
	const std::string getName() const { return meshName; }

private:
	//The parent model manager.
	ModelManager* manager;
	//The mesh this object is referencing.
	Mesh* mesh;
	//The name of the referenced mesh.
	std::string meshName;
	//The cache level the reference requires the mesh to be at.
	CacheLevel level;
};
