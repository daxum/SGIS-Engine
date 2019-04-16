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

#include "Renderer/ShaderInfo.hpp"
#include "Renderer/Std140Aligner.hpp"

class ModelManager;

//Represents a material used to render a mesh. For now, all materials are created at loading time.
class Material {
public:
	/**
	 * Creates a new material.
	 * @param name The name of the material.
	 * @param shader The material's shader.
	 * @param uniformSet The name of the uniform set used in the material.
	 * @param uniformData Values for the buffered uniforms in the material.
	 * @param viewCull Whether to use view culling with this material.
	 */
	Material(const std::string& name, const std::string& shader, const std::string& uniformSet, const UniformSet& uniformSetLayout, bool viewCull = true);

	//Name of the material.
	std::string name;
	//The shader the material uses.
	std::string shader;
	//Name of the uniform set the material uses.
	std::string uniformSet;
	//Uniform data for the material.
	Std140Aligner uniforms;
	//Whether the material has uniforms that belong in a uniform buffer.
	bool hasBufferedUniforms;
	//The textures the material uses, in binding order.
	std::vector<std::string> textures;
	//Whether to use view culling with the material.
	bool viewCull;

	//Amount of references this material has.
	size_t references;
	//Location of the material's uniform data in its uniform buffer.
	//Only used by rendering engine.
	size_t uniformOffset;
};

class MaterialRef {
public:
	/**
	 * Creates a reference to the given material.
	 * @param manager The model manager that created this reference.
	 * @param materialName The name of the material.
	 * @param model The material to reference.
	 */
	MaterialRef(ModelManager* manager, const std::string& materialName, Material* material) :
		manager(manager),
		material(material),
		materialName(materialName) {}

	/**
	 * Decrements the material's reference count.
	 */
	~MaterialRef();

	/**
	 * Returns the material this reference is referencing. The returned
	 * reference is only guaranteed to have the same lifespan as the
	 * reference object it was retrieved from.
	 */
	const Material* getMaterial() const { return material; }

private:
	//The parent model manager.
	ModelManager* manager;
	//The model this object is referencing.
	Material* material;
	//The name of the referenced model.
	std::string materialName;
};
