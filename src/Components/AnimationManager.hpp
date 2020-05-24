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

#include "ComponentManager.hpp"
#include "AnimationComponent.hpp"

class AnimationManager : public ComponentManager {
public:
	AnimationManager() : ComponentManager(ANIMATION_COMPONENT_NAME) {}

	void update() override {
		for (std::shared_ptr<Component> comp : components) {
			std::static_pointer_cast<AnimationComponent>(comp)->update();
		}
	}
};
