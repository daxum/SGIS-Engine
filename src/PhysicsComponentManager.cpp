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
		std::shared_ptr<PhysicsComponent> physics = std::static_pointer_cast<PhysicsComponent>(comp);

		//Actual physics engine to come at later date.
		AxisAlignedBB& box = physics->getBox();
		glm::vec3& velocity = physics->getVelocity();

		box.translate(velocity);
		velocity *= 0.98f;

		//Temporary bounds checking - find a better way later.
		glm::vec3 correction;

		if (box.min.x < border.min.x || box.max.x > border.max.x) {
			correction.x = ExMath::minMagnitude(border.min.x - box.min.x, border.max.x - box.max.x);
			velocity.x = 0.0f;
		}

		if (box.min.y < border.min.y || box.max.y > border.max.y) {
			correction.y = ExMath::minMagnitude(border.min.y - box.min.y, border.max.y - box.max.y);
			velocity.y = 0.0f;
		}

		if (box.min.z < border.min.z || box.max.z > border.max.z) {
			correction.z = ExMath::minMagnitude(border.min.z - box.min.z, border.max.z - box.max.z);
			velocity.z = 0.0f;
		}

		box.translate(correction);
	}
}
