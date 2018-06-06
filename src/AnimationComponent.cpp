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

#include "AnimationComponent.hpp"
#include "Object.hpp"

AnimationComponent::AnimationComponent(const std::vector<std::pair<glm::vec3, glm::quat>>& frames, const float time, const glm::vec3& offset, const glm::mat4& matrix) :
	Component(ANIMATION_COMPONENT_NAME),
	animation(frames, time, matrix),
	time(0),
	posOffset(offset) {

}

void AnimationComponent::onParentSet() {
	lockParent()->setPhysics(this);
}

glm::vec3 AnimationComponent::getTranslation() {
	return animation.getLocation((float)time).first + posOffset;
}

glm::quat AnimationComponent::getRotation() {
	return animation.getLocation((float)time).second;
}
