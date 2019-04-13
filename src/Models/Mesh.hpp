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
	 * @param vertices The vertices for the mesh. Their internal data is copied into the mesh.
	 * @param indices The index data for the mesh.
	 * TODO: calculate two below in constructor.
	 * @param box The bounding box for the mesh.
	 * @param radius The radius of the mesh.
	 */
	Mesh(BufferInfo bufferInfo, const VertexFormat* format, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const Aabb<float>& box, float radius);

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
		return {vertexData, vertexSize, indices};
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
	 * Gets the data needed to render the mesh. The pair contains the index offset in
	 * first, and the index count in second.
	 * @return The index info needed to render the mesh.
	 */
	std::pair<uintptr_t, uint32_t> getRenderInfo() { return {indexStart, indices.size()}; }

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
			bufferInfo = mesh.bufferInfo;
			format = mesh.format;
			box = mesh.box;
			radius = mesh.radius;
			indexStart = mesh.indexStart;
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
			bufferInfo = std::move(mesh.bufferInfo);
			format = std::exchange(mesh.format, nullptr);
			box = std::move(mesh.box);
			radius = std::exchange(mesh.radius, 0.0f);
			indexStart = std::exchange(mesh.indexStart, 0);
		}

		return *this;
	}

private:
	//The mesh's vertex data and size.
	unsigned char* vertexData;
	size_t vertexSize;

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
