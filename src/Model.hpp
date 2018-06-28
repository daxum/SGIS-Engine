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
#include <memory>

#include "AxisAlignedBB.hpp"

//Object containing renderer-specific information for the mesh.
class RenderMeshObject {
public:
	virtual ~RenderMeshObject() {}
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
	Model(std::shared_ptr<RenderMeshObject> mesh, AxisAlignedBB meshBox, float radius, std::string texture, std::string shader, LightInfo light, bool viewCull = true);

	/**
	 * Casts the mesh object to the specified type.
	 * Only should be called from the rendering engine.
	 * @return The mesh object cast to the given type.
	 */
	template<typename T> std::shared_ptr<T> getMesh() {
		return std::static_pointer_cast<T>(mesh);
	}

	std::shared_ptr<RenderMeshObject> mesh;
	std::string texture;
	AxisAlignedBB meshBox;
	float radius;
	std::string shader;
	LightInfo lighting;
	bool viewCull;
};
