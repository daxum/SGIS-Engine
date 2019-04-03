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

#include "ShaderInfo.hpp"
#include "Std140Aligner.hpp"

class ModelManager;

class MeshRef {
public:
	/**
	 * Creates a reference to the given mesh.
	 * @param manager The model manager that created this reference.
	 * @param meshName The name of the referenced mesh.
	 * @param mesh The mesh to reference.
	 * @param level The required cache level for the mesh.
	 */
	MeshRef(ModelManager* manager, const std::string& meshName, Mesh& mesh, CacheLevel level);

	/**
	 * Decrements the mesh's reference count.
	 */
	~MeshRef();

	/**
	 * Returns the mesh this reference is referencing. The returned
	 * pointer is only guaranteed to have the same lifespan as the
	 * reference object it was retrieved from.
	 */
	const Mesh& getMesh() const { return mesh; }

	/**
	 * Returns the name of the mesh.
	 * @return The mesh's name.
	 */
	const std::string getName() const { return meshName; }

private:
	//The parent model manager.
	ModelManager* manager;
	//The mesh this object is referencing.
	Mesh& mesh;
	//The name of the referenced mesh.
	std::string meshName;
	//The cache level the reference requires the mesh to be at.
	CacheLevel level;
};

class Model {
public:
	/**
	 * Creates a new model.
	 * @param modelName The name of the model.
	 * @param mesh The name of the model's mesh.
	 * @param shader The model's shader.
	 * @param uniformSet The name of the uniform set used in the model.
	 * @param uniforms The uniform set for the model, used to determine uniform buffer layout.
	 * @param viewCull Whether to use view culling on this model.
	 */
	Model(const std::string& name, const std::string& mesh, const std::string& shader, const std::string& uniformSet, const UniformSet& uniforms, bool viewCull = true);

	//TODO: This should only be needed by the Vk renderer, remove once that's fixed.
	std::string name;
	//The name of this model's mesh.
	std::string mesh;
	//The shader the model uses.
	std::string shader;
	//Name of the uniform set the model uses.
	std::string uniformSet;
	//Whether the model has uniforms that belong in a uniform buffer.
	bool hasBufferedUniforms;
	//The textures the model uses, in binding order.
	std::vector<std::string> textures;
	//Whether to use view culling on the model.
	bool viewCull;

	//Amount of references this model has.
	size_t references;

	/**
	 * Gets an interface into the model's currently set uniform data.
	 * Only the const version is threadsafe.
	 * @return An aligner for this model.
	 */
	Std140Aligner getAligner() { return uniforms.getAligner(uniformData.get()); }
	const Std140Aligner getAligner() const { return uniforms.getAligner(uniformData.get()); }

	/**
	 * Gets the model's uniform data, for uploading to the rendering engine.
	 * @return A pair consisting of the raw data and size of the data.
	 */
	std::pair<const unsigned char*, size_t> getUniformData() const { return {uniformData.get(), uniforms.getUniformDataSize()}; }

private:
	//TODO: Rectify temporary hack.
	struct ArrayDeleter {
		void operator()(unsigned char* p) {
			delete[] p;
		}
	};

	//Uniform data for the model, aligned to std140 rules.
	Std140AlignerFactory uniforms;
	//Only shallow copies for now, pretty sure this is read only anyway.
	std::shared_ptr<unsigned char> uniformData;
};

class ModelRef {
public:
	/**
	 * Creates a reference to the given model.
	 * @param manager The model manager that created this reference.
	 * @param model The model to reference.
	 * @param mesh The mesh the model uses.
	 */
	ModelRef(ModelManager* manager, std::string modelName, Model& model, std::shared_ptr<MeshRef> mesh);

	/**
	 * Decrements the model's reference count.
	 */
	~ModelRef();

	/**
	 * Returns the model this reference is referencing. The returned
	 * reference is only guaranteed to have the same lifespan as the
	 * reference object it was retrieved from.
	 */
	const Model& getModel() const { return model; }

	/**
	 * Returns the model's mesh.
	 * @return The mesh.
	 */
	const Mesh& getMesh() const { return mesh->getMesh(); }

private:
	//The parent model manager.
	ModelManager* manager;
	//The model this object is referencing.
	Model& model;
	//The mesh for the model.
	std::shared_ptr<MeshRef> mesh;
	//The name of the referenced model.
	std::string modelName;
};
