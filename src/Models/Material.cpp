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

#include "Material.hpp"
#include "ModelManager.hpp"

namespace {
	//Needed for model's constructor, maybe make member function?
	//Strips out all uniforms that don't belong in the model's uniform buffer.
	std::vector<UniformDescription> stripNonBufferedModel(const UniformSet& uniformSet) {
		std::vector<UniformDescription> out;

		for (const UniformDescription& uniform : uniformSet.uniforms) {
			if (uniform.provider == UniformProviderType::MATERIAL && !isSampler(uniform.type)) {
				out.push_back(uniform);
			}
		}

		return out;
	}
}

Material::Material(const std::string& name, const std::string& shader, const std::string& uniformSet, const Std140Aligner& uniformData, bool viewCull)  :
	name(name),
	shader(shader),
	uniformSet(uniformSet),
	uniforms(uniformData),
	hasBufferedUniforms(false),
	textures(),
	viewCull(viewCull),
	references(0) {

	for (const UniformDescription& descr : uniforms.uniforms) {
		if (!isSampler(descr.type)) {
			hasBufferedUniforms = true;
			break;
		}
	}
}

MaterialRef::~MaterialRef()  {
	manager->removeMaterialReference(materialName);
}
