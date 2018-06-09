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
#include "GlShaderLoader.hpp"
#include "RenderComponentManager.hpp"
#include "RenderComponent.hpp"
#include "ExtraMath.hpp"
#include "Camera.hpp"

namespace {
	//Bit of a hack for static callbacks
	GlRenderingEngine* renderer = nullptr;
	DisplayEngine* display = nullptr;
}

GlRenderingEngine::GlRenderingEngine(ModelManager& modelManager, const LogConfig& rendererLog, const LogConfig& loaderLog) :
	RenderingEngine(std::make_shared<GlTextureLoader>(loaderLogger, textureMap),
					std::make_shared<GlShaderLoader>(loaderLogger, shaderMap, textureMap)),
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
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	std::vector<GLuint> namesToDelete(textureMap.size());

	size_t i = 0;
	for (const auto& tex : textureMap) {
		namesToDelete[i] = tex.second.id;
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
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);

	glClearColor(0.0, 0.2, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	memoryManager.init();

	logger.info("OpenGL initialization complete.");
}

RendererMemoryManager* GlRenderingEngine::getMemoryManager() {
	return &memoryManager;
}

void GlRenderingEngine::finishLoad() {
	memoryManager.upload();
}

void GlRenderingEngine::render(std::shared_ptr<RenderComponentManager> data, std::shared_ptr<Camera> camera, std::shared_ptr<ScreenState> state) {
	//Don't render without a render component.
	if (!data) {
		return;
	}

	MatrixStack matStack;

	matStack.multiply(camera->getView());
	const auto nearFar = camera->getNearFar();
	const auto cameraBox = getCameraCollisionData(camera->getView(), camera->getProjection(), nearFar.first, nearFar.second);

	//TODO: sort properly so this isn't needed.
	MeshType currentBuffer = MeshType::BUFFER_COUNT;

	//Render all objects.
	//Also, don't ask what the auto actually is. Just don't.
	for (auto& object : data->getComponentShaderMap()) {
		if (object.second.size() == 0) {
			continue;
		}

		std::shared_ptr<Shader> shader = shaderMap.at(object.first);
		shader->getRenderInterface()->bind();

		shader->setGlobalUniforms(camera, state);

		for (std::shared_ptr<RenderComponent> renderComponent : object.second) {
			const Model& model = renderComponent->getModel();
			const glm::vec3 scale = renderComponent->getScale();
			float maxScale = std::max({scale.x, scale.y, scale.z});

			const std::pair<glm::vec3, float> sphere = std::make_pair(renderComponent->getTranslation(), model.radius * maxScale);

			if (!model.viewCull || checkVisible(sphere, cameraBox)) {
				if (model.mesh.type != currentBuffer) {
					memoryManager.bindBuffer(model.mesh.type);
					currentBuffer = model.mesh.type;
				}

				renderObject(matStack, shader, renderComponent, state);
			}
		}
	}

	glBindVertexArray(0);
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

float GlRenderingEngine::getWindowWidth() const {
	return width;
}

float GlRenderingEngine::getWindowHeight() const {
	return height;
}

glm::vec2 GlRenderingEngine::queryMousePos() const {
	double x, y;
	glfwGetCursorPos(window, &x, &y);

	return glm::vec2(x, y);
}

void GlRenderingEngine::renderObject(MatrixStack& matStack, std::shared_ptr<Shader> shader, std::shared_ptr<RenderComponent> data, std::shared_ptr<ScreenState> state) {
	matStack.push();

	shader->setPerObjectUniforms(data, matStack, state);

	const Model& model = data->getModel();
	glDrawElements(GL_TRIANGLES, model.mesh.indexCount, GL_UNSIGNED_INT, (void*) model.mesh.indexStart);

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

std::vector<std::pair<glm::vec3, glm::vec3>> GlRenderingEngine::getCameraCollisionData(glm::mat4 view, glm::mat4 projection, float nearPlane, float farPlane) {
	const RenderConfig renderConfig = Engine::instance->getConfig().renderer;

	auto topLeft = ExMath::screenToWorld(glm::vec2(0.0, 0.0), projection, view, width, height, nearPlane, farPlane);
	auto topRight = ExMath::screenToWorld(glm::vec2(width, 0.0), projection, view, width, height, nearPlane, farPlane);
	auto bottomLeft = ExMath::screenToWorld(glm::vec2(0.0, height), projection, view, width, height, nearPlane, farPlane);
	auto bottomRight = ExMath::screenToWorld(glm::vec2(width, height), projection, view, width, height, nearPlane, farPlane);

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

void GlRenderingEngine::setViewport(GLFWwindow* window, int nWidth, int nHeight) {
	renderer->width = (float) nWidth;
	renderer->height = (float) nHeight;

	glViewport(0, 0, nWidth, nHeight);
	display->updateProjections();
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
