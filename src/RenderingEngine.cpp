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

#include "RenderingEngine.hpp"
#include "Engine.hpp"
#include "ExtraMath.hpp"

void RenderingEngine::render(std::shared_ptr<RenderComponentManager> renderManager, std::shared_ptr<Camera> camera, std::shared_ptr<ScreenState> state) {
	//Don't render without render component
	if (!renderManager) {
		return;
	}

	//View culling to check if objects are within the camera's view

	const std::unordered_set<std::shared_ptr<RenderComponent>>& components = renderManager->getComponentSet();
	tbb::concurrent_unordered_set<RenderComponent*> visibleComponents;

	const float width = getWindowInterface().getWindowWidth();
	const float height = getWindowInterface().getWindowHeight();

	glm::mat4 projection = camera->getProjection();
	glm::mat4 view = camera->getView();
	float fov = camera->getFOV();
	float nearDist = camera->getNearFar().first;
	float farDist = camera->getNearFar().second;

	glm::vec3 corner = ExMath::screenToWorld(glm::vec2(0.0, 0.0), projection, view, width, height, nearDist, farDist).first;
	glm::vec3 center = ExMath::screenToWorld(glm::vec2(width / 2.0f, height / 2.0f), projection, view, width, height, nearDist, farDist).first;

	float cameraRadius = glm::distance(corner, center);

	Engine::instance->parallelFor(0, components.size(), [&](size_t index) {
		auto iter = components.begin();

		//This seems slow, but it's probably faster than copying everything into a vector.
		for (size_t i = 0; i < index; i++) {
			iter++;
		}

		if (!((*iter)->getModel()->getModel().viewCull) || checkVisible(cameraRadius, view, *iter, fov, nearDist, farDist)) {
			visibleComponents.insert((*iter).get());
		}
	});

	//Render all visible objects

	renderObjects(visibleComponents, renderManager->getComponentList(), camera, state);
}

bool RenderingEngine::checkVisible(float cameraRadius, const glm::mat4& viewMat, std::shared_ptr<RenderComponent> object, float fov, float nearDist, float farDist) {
	glm::vec3 objectPosCamera = glm::vec3(viewMat * glm::vec4(object->getTranslation(), 1.0f));
	float objectRadius = object->getModel()->getMesh().getRadius();

	//If object is behind near plane or beyond far plane, it can't be seen
	if ((objectPosCamera.z - objectRadius) > nearDist || (objectPosCamera.z + objectRadius) < farDist) {
		return false;
	}

	//If object intersects camera, it's visible
	if (glm::length(objectPosCamera) <= (cameraRadius + objectRadius)) {
		return true;
	}

	//If field of view 0, orthographic projection. Check if object intersects camera circle in x and y directions
	if (fov == 0.0f) {
		return glm::length(glm::vec3(objectPosCamera.x, objectPosCamera.y, 0.0f)) <= (cameraRadius + objectRadius);
	}

	//Check against fov

	glm::vec2 unitPos = glm::normalize(objectPosCamera);

	//Translate towards camera by radius amount in the x and y directions
	objectPosCamera -= glm::vec3(unitPos * objectRadius, 0.0f);

	//Translate away from camera by the radius of the near plane's bounding box
	glm::vec3 posStart = glm::vec3(unitPos * cameraRadius, 0.0f);

	//Test if angle of vector in x and y directions is greater than field of view
	glm::vec3 posVec = objectPosCamera - posStart;

	float angleX = asin(posVec.y / glm::length(posVec));
	float angleY = asin(posVec.x / glm::length(posVec));

	return angleX <= fov && angleY <= fov;
}
