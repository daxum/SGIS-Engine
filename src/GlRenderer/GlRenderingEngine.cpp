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

#include <vector>
#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

#include "Engine.hpp"
#include "EngineConfig.hpp"
#include "DisplayEngine.hpp"
#include "GlfwKeyTranslator.hpp"
#include "GlRenderingEngine.hpp"
#include "GlTextureLoader.hpp"
#include "GlShaderLoader.hpp"
#include "RenderComponentManager.hpp"
#include "RenderComponent.hpp"
#include "ExtraMath.hpp"

namespace {
	//Bit of a hack for static callbacks
	GlRenderingEngine* renderer = nullptr;
	DisplayEngine* display = nullptr;
}

GlRenderingEngine::GlRenderingEngine(ModelManager& modelManager, const LogConfig& rendererLog, const LogConfig& loaderLog) :
	RenderingEngine(std::make_shared<GlTextureLoader>(loaderLogger, textureMap),
					std::make_shared<GlShaderLoader>(loaderLogger, shaderMap)),
	logger(rendererLog.type, rendererLog.mask, rendererLog.outputFile),
	loaderLogger(loaderLog.type, loaderLog.mask, loaderLog.outputFile),
	modelManager(modelManager),
	width(0.0f),
	height(0.0f),
	memoryManager(logger) {

	if (renderer != nullptr) {
		throw std::runtime_error("GlRenderingEngine initialized more than once!");
	}

	renderer = this;

	glfwSetErrorCallback(GlRenderingEngine::glfwError);

	if (!glfwInit()) {
		throw std::runtime_error("Couldn't initialize glfw");
	}
}

GlRenderingEngine::~GlRenderingEngine() {
	//Delete textures

	glBindTexture(GL_TEXTURE_2D, 0);

	std::vector<GLuint> namesToDelete(textureMap.size());

	size_t i = 0;
	for (const auto& tex : textureMap) {
		namesToDelete[i] = tex.second;
		i++;
	}

	glDeleteTextures(textureMap.size(), namesToDelete.data());

	//Delete window and terminate glfw

	if (window != nullptr) {
		glfwDestroyWindow(window);
		window = nullptr;
	}

	glfwTerminate();
	renderer = nullptr;
	display = nullptr;

	logger.info("Destroyed OpenGL rendering engine.");
}

void GlRenderingEngine::init(int windowWidth, int windowHeight, std::string windowTitle, DisplayEngine* displayEngine) {
	display = displayEngine;

	//Create the window

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	window = glfwCreateWindow(windowWidth, windowHeight, windowTitle.c_str(), nullptr, nullptr);

	if (window == nullptr) {
		throw std::runtime_error("Failed to create window and context");
	}

	glfwMakeContextCurrent(window);

	logger.info("Created window and context.");

	//Load OpenGL functions

	int loadStatus = ogl_LoadFunctions();

	if (loadStatus != 1) {
		throw new std::runtime_error("OpenGl function loading failed");
	}

	logger.info("Loaded all OpenGL functions.");

	//Set callbacks

	glfwSetFramebufferSizeCallback(window, GlRenderingEngine::setViewport);
	glfwSetKeyCallback(window, GlRenderingEngine::keyPress);
	glfwSetCursorPosCallback(window, GlRenderingEngine::mouseMove);
	glfwSetMouseButtonCallback(window, GlRenderingEngine::mouseClick);
	/*
	//Other callbacks - might be moved elsewhere to accommodate engine
	glfwSetScrollCallback(window, mouseScroll);
	*/

	//Set state defaults

	int width = 0;
	int height = 0;

	glfwGetFramebufferSize(window, &width, &height);

	setViewport(nullptr, width, height);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);

	glClearColor(0.0, 0.2, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	logger.info("OpenGL initialization complete.");
}

MeshRenderData GlRenderingEngine::addMesh(ModelData& data, MeshType type) {
	return memoryManager.addMesh(data.vertices, data.indices, type);
}

void GlRenderingEngine::finishLoad() {
	memoryManager.upload();
}

void GlRenderingEngine::render(std::shared_ptr<RenderComponentManager> data, std::shared_ptr<Camera> camera) {
	//Don't render without a render component.
	if (!data) {
		return;
	}

	MatrixStack matStack;

	matStack.multiply(camera->getView());
	const auto cameraBox = getCameraCollisionData(camera->getView());

	//All models use the static buffer at this time
	memoryManager.bindBuffer(MeshType::STATIC);

	unsigned int objectsRendered = 0;
	unsigned int totalObjects = 0;

	//Render all objects.
	//Also, don't ask what the auto actually is. Just don't.
	for (auto& object : data->getComponentShaderMap()) {
		if (object.second.size() == 0) {
			continue;
		}

		std::shared_ptr<GlShader> shader = shaderMap.at(object.first);
		shader->use();

		if (shader->info.projection) {
			shader->setUniformMat4("projection", projection);
		}

		if (shader->info.lightDir) {
			shader->setUniformVec3("lightDir", glm::normalize(glm::vec3(matStack.top() * glm::vec4(1.0, 1.0, 0.01, 0.0))));
		}

		for (std::shared_ptr<RenderComponent> renderComponent : object.second) {
			totalObjects++;
			const Model& model = modelManager.getModel(renderComponent->getModel());
			const glm::vec3 scale = renderComponent->getScale();
			float maxScale = std::max({scale.x, scale.y, scale.z});

			const std::pair<glm::vec3, float> sphere = std::make_pair(renderComponent->getTranslation(), model.radius * maxScale);

			if (checkVisible(sphere, cameraBox)) {
				objectsRendered++;
				renderObject(matStack, shader, renderComponent);
			}
		}
	}

	logger.debug("Rendered " + std::to_string(objectsRendered) + " out of " + std::to_string(totalObjects) + " objects");
}

void GlRenderingEngine::clearBuffers() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void GlRenderingEngine::present() {
	glfwSwapBuffers(window);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

bool GlRenderingEngine::windowClosed() {
	return glfwWindowShouldClose(window);
}

void GlRenderingEngine::pollEvents() {
	glfwPollEvents();
}

void GlRenderingEngine::captureMouse(bool capture) {
	glfwSetInputMode(window, GLFW_CURSOR, capture ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

glm::mat4 GlRenderingEngine::getProjection() const {
	return projection;
}

float GlRenderingEngine::getWindowWidth() const {
	return width;
}

float GlRenderingEngine::getWindowHeight() const {
	return height;
}

void GlRenderingEngine::renderObject(MatrixStack& matStack, std::shared_ptr<GlShader> shader, std::shared_ptr<RenderComponent> data) {
	matStack.push();

	if (shader->info.modelView) {
		matStack.translate(data->getTranslation());
		matStack.rotate(data->getRotation());
		matStack.scale(data->getScale());

		shader->setUniformMat4("modelView", matStack.top());
	}

	if (shader->info.color) {
		shader->setUniformVec3("color", data->getColor());
	}

	const Model& model = modelManager.getModel(data->getModel());

	if (shader->info.lighting) {
		shader->setUniformVec3("ka", model.lighting.ka);
		shader->setUniformVec3("ks", model.lighting.ks);
		shader->setUniformFloat("s", model.lighting.s);
	}

	if (shader->info.tex0) {
		glBindTexture(GL_TEXTURE_2D, textureMap.at(model.texture));
	}

	glDrawElements(GL_TRIANGLES, model.mesh.indexCount, GL_UNSIGNED_INT, (void*) (uintptr_t)model.mesh.indexStart);

	matStack.pop();
}

bool GlRenderingEngine::checkVisible(const std::pair<glm::vec3, float>& sphere, const std::vector<std::pair<glm::vec3, glm::vec3>>& camera) {
	for (const auto& plane : camera) {
		glm::vec3 planeVec = sphere.first - plane.first;
		//Signed distance
		float distance = glm::dot(planeVec, plane.second);

		if (std::abs(distance) > sphere.second && distance > 0.0) {
			return false;
		}
	}

	return true;
}

std::vector<std::pair<glm::vec3, glm::vec3>> GlRenderingEngine::getCameraCollisionData(glm::mat4 view) {
	const RenderConfig renderConfig = Engine::instance->getConfig().renderer;

	auto topLeft = ExMath::screenToWorld(glm::vec2(0.0, 0.0), projection, view, width, height, renderConfig.nearPlane, renderConfig.farPlane);
	auto topRight = ExMath::screenToWorld(glm::vec2(width, 0.0), projection, view, width, height, renderConfig.nearPlane, renderConfig.farPlane);
	auto bottomLeft = ExMath::screenToWorld(glm::vec2(0.0, height), projection, view, width, height, renderConfig.nearPlane, renderConfig.farPlane);
	auto bottomRight = ExMath::screenToWorld(glm::vec2(width, height), projection, view, width, height, renderConfig.nearPlane, renderConfig.farPlane);

	std::vector<std::pair<glm::vec3, glm::vec3>> posNorVec;

	//Near plane
	glm::vec3 pos = ExMath::bilinear3D(std::make_tuple(topLeft.first, topRight.first, bottomLeft.first, bottomRight.first), 0.5f, 0.5f);
	glm::vec3 normal = glm::normalize(glm::cross(topRight.first - pos, topLeft.first - pos));
	posNorVec.push_back(std::make_pair(pos, normal));

	//Left plane (?)
	pos = ExMath::bilinear3D(std::make_tuple(topRight.first, topRight.second, bottomRight.first, bottomRight.second), 0.5f, 0.5f);
	normal = glm::normalize(glm::cross(topRight.second - pos, topRight.first - pos));
	posNorVec.push_back(std::make_pair(pos, normal));

	//Far plane
	pos = ExMath::bilinear3D(std::make_tuple(topLeft.second, topRight.second, bottomLeft.second, bottomRight.second), 0.5f, 0.5f);
	normal = glm::normalize(glm::cross(topLeft.second - pos, topRight.second - pos));
	posNorVec.push_back(std::make_pair(pos, normal));

	//Right plane (?)
	pos = ExMath::bilinear3D(std::make_tuple(topLeft.first, topLeft.second, bottomLeft.first, bottomLeft.second), 0.5f, 0.5f);
	normal = glm::normalize(glm::cross(topLeft.first - pos, topLeft.second - pos));
	posNorVec.push_back(std::make_pair(pos, normal));

	//Top plane
	pos = ExMath::bilinear3D(std::make_tuple(topLeft.first, topRight.first, topLeft.second, topRight.second), 0.5f, 0.5f);
	normal = glm::normalize(glm::cross(topLeft.first - pos, topRight.first - pos));
	posNorVec.push_back(std::make_pair(pos, normal));

	//Bottom plane
	pos = ExMath::bilinear3D(std::make_tuple(bottomLeft.first, bottomRight.first, bottomLeft.second, bottomRight.second), 0.5f, 0.5f);
	normal = glm::normalize(glm::cross(bottomRight.first - pos, bottomLeft.first - pos));
	posNorVec.push_back(std::make_pair(pos, normal));

	return posNorVec;
}

void GlRenderingEngine::setProjection(int width, int height) {
	const RenderConfig renderConfig = Engine::instance->getConfig().renderer;
	projection = glm::perspective(PI / 4.0f, (float)width / height, renderConfig.nearPlane, renderConfig.farPlane);
}

void GlRenderingEngine::setViewport(GLFWwindow* window, int nWidth, int nHeight) {
	renderer->width = (float) nWidth;
	renderer->height = (float) nHeight;

	int adjustedHeight = std::max(9*nWidth/16, nHeight);
	int heightOffset = -(adjustedHeight - nHeight) / 2;

	glViewport(0, heightOffset, nWidth, adjustedHeight);
	renderer->setProjection(nWidth, adjustedHeight);
}

void GlRenderingEngine::glfwError(int errorCode, const char* description) {
	throw std::runtime_error("GLFW error! " + std::to_string(errorCode) + ": " + description);
}

void GlRenderingEngine::keyPress(GLFWwindow* window, int key, int scancode, int action, int mods) {
	KeyAction nativeAction;

	switch (action) {
		case GLFW_PRESS: nativeAction = KeyAction::PRESS; break;
		case GLFW_REPEAT: nativeAction = KeyAction::REPEAT; break;
		case GLFW_RELEASE: nativeAction = KeyAction::RELEASE; break;
		default: return;
	}

	display->onKeyAction(GLFWKeyTranslator::translate(key), nativeAction);
}

void GlRenderingEngine::mouseMove(GLFWwindow* window, double x, double y) {
	display->onMouseMove((float)x, (float)y);
}

void GlRenderingEngine::mouseClick(GLFWwindow* window, int button, int action, int mods) {
	MouseButton pressed;
	MouseAction mouseAction;

	switch(button) {
		case GLFW_MOUSE_BUTTON_LEFT: pressed = MouseButton::LEFT; break;
		case GLFW_MOUSE_BUTTON_MIDDLE: pressed = MouseButton::MIDDLE; break;
		case GLFW_MOUSE_BUTTON_RIGHT: pressed = MouseButton::RIGHT; break;
		default: return;
	}

	switch(action) {
		case GLFW_PRESS: mouseAction = MouseAction::PRESS; break;
		case GLFW_RELEASE: mouseAction = MouseAction::RELEASE; break;
		default: return;
	}

	display->onMouseClick(pressed, mouseAction);
}
