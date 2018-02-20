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
#include <glm/glm.hpp>
#include "Object.hpp"
#include "RenderData.hpp"

//Describes how to render an object.
class ObjectRenderData : public RenderData {
public:
	/**
	 * Creates rendering data for the passed in object.
	 * @param parent The object this rendering data is for.
	 * @param model The name of the model to use to render this object.
	 */
	ObjectRenderData(Object* parent, std::string model);

	/**
	 * Returns the translation of this object.
	 * @return A translation vector for this object.
	 */
	glm::vec3 getTranslation();

	/**
	 * Returns the rotation of this object.
	 * @return A rotation vector for this object.
	 */
	glm::vec3 getRotation();

	/**
	 * Returns the scale of this object.
	 * @return The scale of this object.
	 */
	glm::vec3 getScale();

	/**
	 * Returns the name of the model to be used in rendering this object.
	 * @return The model name.
	 */
	std::string getModel() { return model; }

private:
	//The object this rendering data is for.
	Object* parent;

	//Which model to use for this object.
	std::string model;
};