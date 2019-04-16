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

#include <algorithm>

#include "RenderingEngine.hpp"
#include "Engine.hpp"
#include "ExtraMath.hpp"

void RenderingEngine::render(const Screen* screen) {
	std::shared_ptr<const RenderComponentManager> renderManager = screen->getRenderData();

	//Don't render without render component
	if (!renderManager) {
		return;
	}

	//View culling to check if objects are within the camera's view

	const std::vector<const RenderComponent*>& componentVec = renderManager->getComponentSet();

	const float width = getWindowInterface().getWindowWidth();
	const float height = getWindowInterface().getWindowHeight();

	std::shared_ptr<const Camera> camera = screen->getCamera();

	glm::mat4 projection = camera->getProjection();
	glm::mat4 view = camera->getView();
	float nearDist = camera->getNearFar().first;
	float farDist = camera->getNearFar().second;

	const std::array<std::pair<glm::vec2, glm::vec2>, 4> cameraBox = {
		//Top left
		ExMath::screenToWorld(glm::vec2(0.0, 0.0), projection, glm::mat4(1.0f), width, height, nearDist, farDist),
		//Top right
		ExMath::screenToWorld(glm::vec2(width, 0.0), projection, glm::mat4(1.0f), width, height, nearDist, farDist),
		//Bottom left
		ExMath::screenToWorld(glm::vec2(0.0, height), projection, glm::mat4(1.0f), width, height, nearDist, farDist),
		//Bottom right
		ExMath::screenToWorld(glm::vec2(width, height), projection, glm::mat4(1.0f), width, height, nearDist, farDist)
	};

	Engine::instance->parallelFor(0, componentVec.size(), [&](size_t index) {
		componentVec.at(index)->setVisible(
			!(componentVec.at(index)->getModel()->getModel().viewCull) ||
			checkVisible(cameraBox, view, componentVec.at(index), nearDist, farDist)
		);
	});

	//Render all visible objects

	renderObjects(renderManager->getComponentList(), screen);
}

bool RenderingEngine::checkVisible(const std::array<std::pair<glm::vec2, glm::vec2>, 4>& cameraBox, const glm::mat4& viewMat, const RenderComponent* object, float nearDist, float farDist) {
	const float near = -nearDist;
	const float far = -farDist;

	const glm::vec3 objectPosCamera = viewMat * glm::vec4(object->getTranslation(), 1.0f);
	const float objectRadius = object->getModel()->getMesh().getRadius() * std::max({object->getScale().x, object->getScale().y, object->getScale().z});

	//Object behind near plane or beyond far plane
	if ((objectPosCamera.z - objectRadius) > near || (objectPosCamera.z + objectRadius) < far) {
		return false;
	}

	//Object intersects near plane
	if (((objectPosCamera.z + objectRadius) < near) != ((objectPosCamera.z - objectRadius) < near)) {
		const float objectDist = near - objectPosCamera.z;
		const float objectRadiusPlane = sqrt(objectRadius * objectRadius + objectDist * objectDist);

		//Create bounding box from object sphere (top, bottom, right, left)
		const glm::vec4 objectBoxSides(
			objectPosCamera.y + objectRadiusPlane,
			objectPosCamera.y - objectRadiusPlane,
			objectPosCamera.x + objectRadiusPlane,
			objectPosCamera.x - objectRadiusPlane
		);

		//Create bounding box from camera near plane
		const glm::vec4 cameraBoxSides(
			cameraBox.at(0).first.y,
			cameraBox.at(2).first.y,
			cameraBox.at(1).first.x,
			cameraBox.at(0).first.x
		);

		return objectBoxSides.y < cameraBoxSides.x &&
			   objectBoxSides.x > cameraBoxSides.y &&
			   objectBoxSides.w < cameraBoxSides.z &&
			   objectBoxSides.z > cameraBoxSides.w;
	}

	//Object intersects far plane
	if (((objectPosCamera.z + objectRadius) < far) != ((objectPosCamera.z - objectRadius) < far)) {
		const float objectDist = far - objectPosCamera.z;
		const float objectRadiusPlane = sqrt(objectRadius * objectRadius + objectDist * objectDist);

		//Create bounding box from object sphere (top, bottom, right, left)
		const glm::vec4 objectBoxSides(
			objectPosCamera.y + objectRadiusPlane,
			objectPosCamera.y - objectRadiusPlane,
			objectPosCamera.x + objectRadiusPlane,
			objectPosCamera.x - objectRadiusPlane
		);

		//Create bounding box from camera far plane
		const glm::vec4 cameraBoxSides(
			cameraBox.at(0).second.y,
			cameraBox.at(2).second.y,
			cameraBox.at(1).second.x,
			cameraBox.at(0).second.x
		);

		return objectBoxSides.y < cameraBoxSides.x &&
			   objectBoxSides.x > cameraBoxSides.y &&
			   objectBoxSides.w < cameraBoxSides.z &&
			   objectBoxSides.z > cameraBoxSides.w;
	}

	const float percent = (near + objectPosCamera.z) / far;

	//Width and height of the plane parallel to the near and far planes with same z coordinate as the object
	const glm::vec2 cameraPlaneSize = {
		std::abs(ExMath::interpolate<glm::vec2>(cameraBox.at(1).first, cameraBox.at(1).second, percent).x),
		std::abs(ExMath::interpolate<glm::vec2>(cameraBox.at(1).first, cameraBox.at(1).second, percent).y)
	};

	return (std::abs(objectPosCamera.x) - objectRadius) <= cameraPlaneSize.x && (std::abs(objectPosCamera.y) - objectRadius) <= cameraPlaneSize.y;
}
