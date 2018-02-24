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

#include <glm/glm.hpp>

//Describes how to render an arbitrary thing.
class RenderData {
public:
	virtual ~RenderData() {}

	/**
	 * Returns the translation.
	 * @return A translation vector.
	 */
	virtual glm::vec3 getTranslation() = 0;

	/**
	 * Returns the rotation.
	 * @return A rotation vector.
	 */
	virtual glm::vec3 getRotation() = 0;

	/**
	 * Returns the scale.
	 * @return The scale.
	 */
	virtual glm::vec3 getScale() = 0;

	/**
	 * Returns the name of the model to be used in rendering.
	 * @return The model name.
	 */
	virtual std::string getModel() = 0;
};

