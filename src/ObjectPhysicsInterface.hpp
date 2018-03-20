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

//Used to prevent the need for components to directly access physics components for objects,
//especially for objects that shouldn't have fully simulated physics. Will allow for different
//"physics providers" as well - could theoretically be used in a gui or similar.
class ObjectPhysicsInterface {
public:
	/**
	 * Returns the translation of the object in the world.
	 */
	virtual glm::vec3 getTranslation() { return glm::vec3(0.0, 0.0, 0.0); }

	/**
	 * Returns the rotation as angles around the x, y, and z axis.
	 */
	virtual glm::vec3 getRotation() { return glm::vec3(0.0, 0.0, 0.0); }
};
