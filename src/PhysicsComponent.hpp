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

#include "Component.hpp"

class PhysicsComponent : public Component {
public:
	/**
	 * Creates a PhysicsComponent with the provided object as its parent
	 */
	PhysicsComponent(Object& object) : Component(object) {}

	/**
	 * Gets the object associated with this physics component.
	 * Used by the physics engine for direct access to the position,
	 * velocity, etc.
	 * @return The object that owns this component.
	 */
	Object& getObject() { return parent; }
};
