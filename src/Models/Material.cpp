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

Material::Material(const std::string& name, const std::string& shader, const std::string& uniformSet, const UniformSet& uniformSetLayout, bool viewCull)  :
	name(name),
	shader(shader),
	uniformSet(uniformSet),
	uniforms(uniformSetLayout.getBufferedUniforms()),
	hasBufferedUniforms(!uniformSetLayout.getBufferedUniforms().empty()),
	textures(),
	viewCull(viewCull),
	references(0) {

}

MaterialRef::~MaterialRef()  {
	manager->removeMaterialReference(materialName);
}
