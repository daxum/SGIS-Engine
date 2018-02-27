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

#include "PhysicsComponentManager.hpp"
#include "PhysicsComponent.hpp"

void PhysicsComponentManager::update(Screen* screen) {
	for (std::shared_ptr<Component> comp : components) {
		Object& object = std::static_pointer_cast<PhysicsComponent>(comp)->getObject();

		//Actual physics engine to come at later date.
		object.box.translate(object.velocity);
		object.velocity *= 0.98f;
	}
}
