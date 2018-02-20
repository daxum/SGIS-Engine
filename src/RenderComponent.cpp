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

#include "RenderComponent.hpp"

RenderComponent::RenderComponent(Object& parent, std::string model) :
	Component(parent),
	model(model) {

}

glm::vec3 RenderComponent::getTranslation() {
	return parent.position();
}

glm::vec3 RenderComponent::getRotation() {
	return glm::vec3(0.0f, 0.0f, 0.0f);
}

glm::vec3 RenderComponent::getScale() {
	return glm::vec3(1.0f, 1.0f, 1.0f);
}
