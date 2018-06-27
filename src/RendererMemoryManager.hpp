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

#include "Model.hpp"
#include "Vertex.hpp"

//An interface to the rendering engine's memory manager.
class RendererMemoryManager {
public:
	/**
	 * Destructor.
	 */
	virtual ~RendererMemoryManager() {}

	/**
	 * Adds a static mesh (vertices + indices) to be uploaded to the gpu at a later time.
	 * @param vertices The vertices for the mesh to be uploaded.
	 * @param indices The vertex indices for the mesh to be uploaded.
	 * @return a MeshData struct that can be used for drawing.
	 * @throw runtime_error if the memory manager has already been initialized.
	 */
	virtual std::shared_ptr<RenderMeshObject> addStaticMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) = 0;

	/**
	 * Dynamically uploads text model data to the gpu. Must call free later!
	 * @param vertices The vertices for the text model.
	 * @param indices The indices for the text model.
	 * @return model data with which to render the text.
	 */
	virtual std::shared_ptr<RenderMeshObject> addTextMesh(const std::vector<TextVertex>& vertices, const std::vector<uint32_t>& indices) = 0;

	/**
	 * Marks the memory previously occupied by the model data as unused.
	 * @param data The model data to free.
	 */
	virtual void freeTextMesh(const MeshRenderData& data) = 0;
};
