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
#include "ExtraMath.hpp"

void PhysicsComponentManager::update(Screen* screen) {
	const AxisAlignedBB& border = screen->getMap()->getBorder();

	for (std::shared_ptr<Component> comp : components) {
		Object& object = std::static_pointer_cast<PhysicsComponent>(comp)->getObject();

		//Actual physics engine to come at later date.
		object.box.translate(object.velocity);
		object.velocity *= 0.98f;

		//Temporary bounds checking - find a better way later.
		glm::vec3 correction;

		if (object.box.min.x < border.min.x || object.box.max.x > border.max.x) {
			correction.x = ExMath::minMagnitude(border.min.x - object.box.min.x, border.max.x - object.box.max.x);
			object.velocity.x = 0.0f;
		}

		if (object.box.min.y < border.min.y || object.box.max.y > border.max.y) {
			correction.y = ExMath::minMagnitude(border.min.y - object.box.min.y, border.max.y - object.box.max.y);
			object.velocity.y = 0.0f;
		}

		if (object.box.min.z < border.min.z || object.box.max.z > border.max.z) {
			correction.z = ExMath::minMagnitude(border.min.z - object.box.min.z, border.max.z - object.box.max.z);
			object.velocity.z = 0.0f;
		}

		object.box.translate(correction);
	}
}
