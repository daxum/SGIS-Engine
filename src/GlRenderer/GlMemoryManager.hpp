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

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <list>

#include "Vertex.hpp"
#include "CombinedGl.h"
#include "Model.hpp"
#include "RendererMemoryManager.hpp"

struct GlBufferData : public RenderBufferData {
	GLuint vertexArray;
	GLuint vertexBufferId;
	GLuint indexBufferId;
	bool useTransfer;

	GlBufferData() :
		vertexArray(0),
		vertexBufferId(0),
		indexBufferId(0),
		useTransfer(false) {}

	~GlBufferData() {
		glDeleteVertexArrays(1, &vertexArray);
		glDeleteBuffers(1, &vertexBufferId);
		glDeleteBuffers(1, &indexBufferId);
	}
};

struct GlMeshRenderData {
	uintptr_t indexStart;
	uint32_t indexCount;
};

class GlMemoryManager : public RendererMemoryManager {
public:
	/**
	 * Initializes the memory manager.
	 * @param logConfig The configuration of the logger with which to log.
	 */
	GlMemoryManager(const LogConfig& logConfig);

	/**
	 * Destructor. Deletes the transfer buffer.
	 */
	~GlMemoryManager();

	/**
	 * Does nothing, as OpenGL doesn't have descriptor sets.
	 */
	void initializeDescriptors() override {}

	/**
	 * Binds the specified buffer for drawing.
	 * @param buffer The buffer to bind.
	 */
	void bindBuffer(const std::string& buffer);

	/**
	 * Returns the data needed to render the mesh with the given name.
	 * @param name The name of the mesh.
	 * @return The render data for the mesh.
	 * @throw std::out_of_range if the mesh isn't present.
	 */
	const GlMeshRenderData& getMeshData(const std::string& name) { return meshData.at(name); }

protected:
	/**
	 * Creates a buffer.
	 * @param vertexFormat The format of the vertices in the buffer.
	 * @param usage The way the buffer is intended to be used. This will use
	 *     GL_STATIC_DRAW for all buffers that should be in dedicated memory,
	 *     and GL_STREAM_DRAW for all buffers that should be in system memory.
	 *     Hopefully the driver cooperates...
	 * @param size The size of the buffer to create.
	 * @return A pointer to a GlBufferData struct that contains the vertex and index buffer ids.
	 * @throw std::runtime_error if out of memory.
	 */
	std::shared_ptr<RenderBufferData> createBuffer(const std::vector<VertexElement>& vertexFormat, BufferUsage usage, size_t size) override;

	/**
	 * Does nothing for now - if a newer OpenGL version is used in the future,
	 * or if the uniform buffer extension is found, this will initialize the buffers.
	 * @param modelStaticSize The size of the static model uniform buffer.
	 * @param modelDynamicSize The size of the dynamic model uniform buffer.
	 * @param screenObjectSize The size of the screen / object uniform buffer.
	 */
	void createUniformBuffers(size_t modelStaticSize, size_t modelDynamicSize, size_t screenObjectSize) override {}

	/**
	 * Minimum uniform buffer alignment, not used until uniform buffers are.
	 * @return 1.
	 */
	virtual size_t getMinUniformBufferAlignment() override { return 1; }

	/**
	 * Uploads the vertex and index data into the given buffer.
	 * @param buffer The buffer to upload to.
	 * @param mesh The name of the mesh being uploaded, used to store rendering data.
	 * @param offset The offset into the vertex buffer to place the vertex data.
	 * @param size The size of the vertex data.
	 * @param vertexData The vertex data to upload.
	 * @param indexOffset The offset into the index buffer to place the index data.
	 * @param indexSize The size of the index data.
	 * @param indexData The index data to upload.
	 */
	void uploadMeshData(const VertexBuffer& buffer, const std::string& mesh, size_t offset, size_t size, const unsigned char* vertexData, size_t indexOffset, size_t indexSize, const uint32_t* indexData) override;

	/**
	 * Removes the mesh's render data from the render map.
	 * @param mesh The name of the mesh to remove.
	 */
	void invalidateMesh(const std::string& mesh) override;

	/**
	 * Some more functions that aren't needed for opengl.
	 */
	void addModelDescriptors(const Model& model) override {}
	void addDynamicDescriptors(const Model& model) override {}
	void removeDynamicDescriptors(const Model& model) override {}

	/**
	 * This one might be used eventually.
	 */
	void uploadModelData(const UniformBufferType buffer, const size_t offset, const size_t size, const unsigned char* data) override {}

private:
	//Stores the rendering data for all uploaded meshes.
	std::unordered_map<std::string, GlMeshRenderData> meshData;
	//Transfer buffer for uploading mesh data to static buffers on the gpu.
	GLuint transferBuffer;
	//Current size of the transfer buffer, will grow for larger meshes.
	size_t transferSize;
};
