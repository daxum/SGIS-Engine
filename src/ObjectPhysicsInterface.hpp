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
#include <glm/gtc/quaternion.hpp>

//Used to prevent the need for components to directly access physics components for objects,
//especially for objects that shouldn't have fully simulated physics. Will allow for different
//"physics providers" as well - could theoretically be used in a gui or similar.
class ObjectPhysicsInterface {
public:
	virtual ~ObjectPhysicsInterface() {}

	/**
	 * Returns the translation of the object in the world.
	 */
	virtual glm::vec3 getTranslation() const { return glm::vec3(0.0, 0.0, 0.0); }

	/**
	 * Returns the rotation of the object.
	 */
	virtual glm::quat getRotation() const { return glm::quat(0.0, glm::vec3(0.0, 0.0, 0.0)); }
};
