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

#include <memory>
#include <string>
#include <cstdint>

#include <glm/glm.hpp>

class ObjectRenderData;

//An object in a world. Stores rendering, physics, etc.
class Object {
public:
	/**
	 * Creates an object.
	 * @param model The name of the model to use when rendering.
	 * @param pos The starting position of the object.
	 */
	Object(std::string model, glm::vec3 pos);

	/**
	 * Returns model used and similar data.
	 */
	std::shared_ptr<ObjectRenderData> getRenderData() { return renderData; }

	/**
	 * Returns the position of this object
	 */
	glm::vec3 position() { return pos; }

	/**
	 * Moves the object to the given position.
	 * @param newPos The new position of the object.
	 */
	void moveTo(glm::vec3 newPos) { pos = newPos; }

	/**
	 * Moves the object by the given amount.
	 * @param amount The amount to move the object by.
	 */
	void move(glm::vec3 amount) { pos += amount; }

private:
	//Has to be pointer or else vector throws a fit.
	std::shared_ptr<ObjectRenderData> renderData;

	//The position of the object in the world.
	glm::vec3 pos;
};
