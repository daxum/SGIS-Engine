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
#include <unordered_map>
#include <memory>

#include "AxisAlignedBB.hpp"
#include "Vertex.hpp"

class ModelManager;

class Mesh {
public:
	/**
	 * Creates a mesh with the given vertices and indices.
	 * @param buffer The buffer to place the mesh in for rendering.
	 * @param vertices The vertices for the mesh. Their internal data is copied into the mesh.
	 * @param indices The index data for the mesh.
	 * TODO: calculate two below in constructor.
	 * @param box The bounding box for the mesh.
	 * @param radius The radius of the mesh.
	 */
	Mesh(const std::string& buffer, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const AxisAlignedBB& box, float radius);

	/**
	 * Gets the buffer the mesh is stored in.
	 * @return the vertex buffer this mesh belongs to.
	 */
	const std::string& getBuffer() const { return buffer; }

	/**
	 * Gets the mesh's bounding box.
	 * @return the bounding box of the mesh.
	 */
	const AxisAlignedBB& getBox() const { return box; }

	/**
	 * Adds a user to this mesh.
	 */
	void addUser() { users++; }

	/**
	 * Removes a user of this mesh.
	 */
	void removeUser() {
		if (users > 0) {
			users--;
		}
	}

	/**
	 * Returns the number of users of this mesh.
	 * @return the number of users.
	 */
	size_t getUsers() { return users; }

	/**
	 * Retrieves all the mesh data, for uploading into a vertex / index buffer.
	 */
	const std::tuple<unsigned char*, size_t, const std::vector<uint32_t>&> getMeshData() {
		return {vertexData, vertexSize, indices};
	}

private:
	//The mesh's vertex data and size.
	unsigned char* vertexData;
	size_t vertexSize;

	//Indices for the vertices.
	std::vector<uint32_t> indices;

	//The buffer this mesh belongs in.
	std::string buffer;

	//Possibly useful dimensions for the mesh, calculated on construction.
	AxisAlignedBB box;
	float radius;

	//How many models use this mesh.
	size_t users;
};

class Model {
public:
	/**
	 * Creates a new model.
	 * @param mesh The name of the model's mesh.
	 * @param shader The model's shader.
	 * @param viewCull Whether to use view culling on this model.
	 */
	Model(std::string mesh, std::string shader, bool viewCull = true) :
		mesh(mesh),
		shader(shader),
		uniformMap(),
		viewCull(viewCull),
		references(0) {}

	//The name of this model's mesh.
	std::string mesh;
	//The shader the model uses.
	std::string shader;
	//A map of uniforms for the model.
	std::unordered_map<std::string, std::shared_ptr<void>> uniformMap;
	//Whether to use view culling on the model.
	bool viewCull;

	//Amount of references this model has.
	size_t references;
};

class ModelRef {
public:
	/**
	 * Creates a reference to the given model.
	 * @param manager The model manager that created this reference.
	 * @param modelName The name of the model this reference is referencing.
	 * @param model The model to reference.
	 * @param mesh The model's mesh, used to avoid repeated lookups from the name
	 *     stored in the model.
	 */
	ModelRef(ModelManager* manager, const std::string& modelName, Model& model, Mesh& mesh);

	/**
	 * Decrements the model's reference count.
	 */
	~ModelRef();

	/**
	 * Returns the model this reference is referencing. The returned
	 * reference is only guarenteed to have the same lifespan as the
	 * reference object it was retrieved from.
	 */
	const Model& getModel() const { return model; }

	/**
	 * Gets the model's mesh, has same restrictions as the model.
	 */
	const Mesh& getMesh() const { return mesh; }

	/**
	 * Gets the name of the model this reference is referencing.
	 */
	const std::string& getName() const { return modelName; }

private:
	//The parent model manager.
	ModelManager* manager;
	//The model this object is referencing.
	Model& model;
	//The model's mesh.
	Mesh& mesh;
	//The name of the above model.
	const std::string& modelName;
};
