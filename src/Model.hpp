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
#include <string>

#include "AxisAlignedBB.hpp"

//The type of mesh, affects mesh storage
enum MeshType {
	//Static meshes will be uploaded to gpu memory, for fast access by the gpu. Their buffers
	//will never be mapped. In addition, their vertices may be combined with the vertices of other
	//static meshes to save memory.
	STATIC = 0,
	//Text meshes.
	DYNAMIC_TEXT,
	//The total number of buffers, not actually a type of buffer.
	BUFFER_COUNT
};

//Contains the data needed to draw a mesh, such as offsets in the index buffer
struct MeshRenderData {
	//The type of this mesh - determines which buffer to use
	MeshType type;
	//The starting position (byte offset) in the index buffer
	size_t indexStart;
	//The number of indices in the mesh
	size_t indexCount;
};

//Which render pass to draw the model in
enum class RenderPass {
	OPAQUE,
	TRANSPARENT,
	TRANSLUCENT
};

struct LightInfo {
	glm::vec3 ka;
	glm::vec3 ks;
	float s;
};

//Stores all model data, like mesh information, lighting parameters, and others.
class Model {
public:
	/**
	 * Constructs a model using the given mesh
	 * @param meshData This model's mesh information
	 * @param meshBox The axis aligned bounding box for this model's mesh.
	 * @param radius The radius of the model.
	 * @param texture The name of the texture to use
	 * @param shader The shader the model uses.
	 * @param pass The render pass to render the model in.
	 */
	Model(MeshRenderData meshData, AxisAlignedBB meshBox, float radius, std::string texture, std::string shader, LightInfo light, RenderPass pass, bool viewCull = true);

	MeshRenderData mesh;
	std::string texture;
	AxisAlignedBB meshBox;
	float radius;
	std::string shader;
	LightInfo lighting;
	RenderPass pass;
	bool viewCull;
};
