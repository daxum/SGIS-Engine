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
#include "GlRenderingEngine.hpp"
#include "GlShaderLoader.hpp"
#include "RenderComponentManager.hpp"
#include "RenderComponent.hpp"
#include "ExtraMath.hpp"
#include "Camera.hpp"

GlRenderingEngine::GlRenderingEngine(DisplayEngine& display, const LogConfig& rendererLog, const LogConfig& loaderLog) :
	RenderingEngine(std::make_shared<GlTextureLoader>(loaderLogger, textureMap),
					std::make_shared<GlShaderLoader>(loaderLogger, &memoryManager, shaderMap, textureMap),
					rendererLog,
					loaderLog),
	interface(display, this),
	memoryManager(rendererLog) {

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

	GLFWwindow* window = interface.getWindow();

	if (window != nullptr) {
		glfwDestroyWindow(window);
	}

	glfwTerminate();

	logger.info("Destroyed OpenGL rendering engine.");
}

void GlRenderingEngine::init() {
	//Create the window

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	const int initWidth = Engine::instance->getConfig().renderer.windowWidth;
	const int initHeight = Engine::instance->getConfig().renderer.windowHeight;
	const std::string& title = Engine::instance->getConfig().renderer.windowTitle;

	GLFWwindow* window = glfwCreateWindow(initWidth, initHeight, title.c_str(), nullptr, nullptr);

	if (window == nullptr) {
		throw std::runtime_error("Failed to create window and context");
	}

	glfwMakeContextCurrent(window);

	logger.info("Created window and context");

	//Load OpenGL functions

	int loadStatus = ogl_LoadFunctions();

	if (loadStatus != 1) {
		throw new std::runtime_error("OpenGl function loading failed");
	}

	logger.info("Loaded all OpenGL functions.");

	//Set callbacks

	interface.init(window);

	//Set viewport

	glViewport(0, 0, (int) interface.getWindowWidth(), (int) interface.getWindowHeight());

	//Set state defaults

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);

	glClearColor(0.0, 0.2, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	logger.info("OpenGL initialization complete.");
}

RendererMemoryManager* GlRenderingEngine::getMemoryManager() {
	return &memoryManager;
}

void GlRenderingEngine::render(std::shared_ptr<RenderComponentManager> data, std::shared_ptr<Camera> camera, std::shared_ptr<ScreenState> state) {
	//Don't render without a render component.
	if (!data) {
		return;
	}

	MatrixStack matStack;

	matStack.multiply(camera->getView());
	const auto nearFar = camera->getNearFar();
	const CameraBox cameraBox = getCameraCollisionData(camera->getView(), camera->getProjection(), nearFar.first, nearFar.second);
	const RenderComponentManager::RenderPassList& transparencyPassList = data->getComponentList();

	//Opaque objects
	renderTransparencyPass(transparencyPassList.at(0), matStack, camera, cameraBox, state);

	//Transparent objects
	renderTransparencyPass(transparencyPassList.at(1), matStack, camera, cameraBox, state);

	//Translucent objects
	renderTransparencyPass(transparencyPassList.at(2), matStack, camera, cameraBox, state, true);

	glBindVertexArray(0);
}

void GlRenderingEngine::clearBuffers() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void GlRenderingEngine::present() {
	glfwSwapBuffers(interface.getWindow());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void GlRenderingEngine::setViewport(int width, int height) {
	glViewport(0, 0, width, height);
}

void GlRenderingEngine::renderObject(MatrixStack& matStack, std::shared_ptr<Shader> shader, std::shared_ptr<RenderComponent> data, std::shared_ptr<ScreenState> state) {
	matStack.push();

	shader->setPerObjectUniforms(data, matStack, state);

	const GlMeshRenderData& renderData = memoryManager.getMeshData(data->getModel()->getModel().mesh);
	glDrawElements(GL_TRIANGLES, renderData.indexCount, GL_UNSIGNED_INT, (void*) renderData.indexStart);

	matStack.pop();
}

void GlRenderingEngine::renderTransparencyPass(const RenderComponentManager::RenderPassObjects& objects, MatrixStack& matStack, std::shared_ptr<Camera> camera, const CameraBox& viewBox, std::shared_ptr<ScreenState> state, bool blend) {
	//TODO
	/*
	bool blendOn = false;

	//For each buffer type.
	for (MeshType i = MeshType::STATIC; i < MeshType::BUFFER_COUNT; i = (MeshType)((int)(i) + 1)) {
		bool bufferBound = false;

		//For each shader used in the current buffer.
		for (const auto& object : objects.at(i)) {
			if (object.second.size() == 0) {
				continue;
			}

			std::shared_ptr<Shader> shader = shaderMap.at(object.first);
			shader->getRenderInterface()->bind();

			shader->setGlobalUniforms(camera, state);

			//For each object that uses the current shader and buffer.
			for (const std::shared_ptr<RenderComponent> renderComponent : object.second) {
				//Check whether the object is visible from the camera (sphere-plane collision with six planes).
				const Model& model = renderComponent->getModel();
				const glm::vec3 scale = renderComponent->getScale();
				const float maxScale = std::max({scale.x, scale.y, scale.z});

				const std::pair<glm::vec3, float> sphere = std::make_pair(renderComponent->getTranslation(), model.radius * maxScale);

				if (!model.viewCull || checkVisible(sphere, viewBox)) {
					//Bind buffer here to avoid binding buffers with no objects using them.
					if (!bufferBound) {
						memoryManager.bindBuffer(i);
						bufferBound = true;
					}

					//Same for blend.
					if (blend && !blendOn) {
						glEnable(GL_BLEND);
						blendOn = true;
					}

					renderObject(matStack, shader, renderComponent, state);
				}
			}
		}
	}

	//Turn off blend if enabled.
	if (blendOn) {
		glDisable(GL_BLEND);
	}*/
}

bool GlRenderingEngine::checkVisible(const std::pair<glm::vec3, float>& sphere, const CameraBox& camera) {
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

GlRenderingEngine::CameraBox GlRenderingEngine::getCameraCollisionData(glm::mat4 view, glm::mat4 projection, float nearPlane, float farPlane) {
	const RenderConfig renderConfig = Engine::instance->getConfig().renderer;
	const float width = interface.getWindowWidth();
	const float height = interface.getWindowHeight();

	auto topLeft = ExMath::screenToWorld(glm::vec2(0.0, 0.0), projection, view, width, height, nearPlane, farPlane);
	auto topRight = ExMath::screenToWorld(glm::vec2(width, 0.0), projection, view, width, height, nearPlane, farPlane);
	auto bottomLeft = ExMath::screenToWorld(glm::vec2(0.0, height), projection, view, width, height, nearPlane, farPlane);
	auto bottomRight = ExMath::screenToWorld(glm::vec2(width, height), projection, view, width, height, nearPlane, farPlane);

	CameraBox posNorVec;

	//Near plane
	glm::vec3 pos = ExMath::bilinear3D(std::make_tuple(topLeft.first, topRight.first, bottomLeft.first, bottomRight.first), 0.5f, 0.5f);
	glm::vec3 normal = glm::normalize(glm::cross(topRight.first - pos, topLeft.first - pos));
	posNorVec[0] = std::make_pair(pos, normal);

	//Right plane
	pos = ExMath::bilinear3D(std::make_tuple(topRight.first, topRight.second, bottomRight.first, bottomRight.second), 0.5f, 0.5f);
	normal = glm::normalize(glm::cross(bottomRight.second - pos, topRight.second - pos));
	posNorVec[1] = std::make_pair(pos, normal);

	//Far plane
	pos = ExMath::bilinear3D(std::make_tuple(topLeft.second, topRight.second, bottomLeft.second, bottomRight.second), 0.5f, 0.5f);
	normal = glm::normalize(glm::cross(topLeft.second - pos, topRight.second - pos));
	posNorVec[2] = std::make_pair(pos, normal);

	//Left plane
	pos = ExMath::bilinear3D(std::make_tuple(topLeft.first, topLeft.second, bottomLeft.first, bottomLeft.second), 0.5f, 0.5f);
	normal = glm::normalize(glm::cross(bottomLeft.first - pos, topLeft.first - pos));
	posNorVec[3] = std::make_pair(pos, normal);

	//Top plane
	pos = ExMath::bilinear3D(std::make_tuple(topLeft.first, topRight.first, topLeft.second, topRight.second), 0.5f, 0.5f);
	normal = glm::normalize(glm::cross(topLeft.first - pos, topRight.first - pos));
	posNorVec[4] = std::make_pair(pos, normal);

	//Bottom plane
	pos = ExMath::bilinear3D(std::make_tuple(bottomLeft.first, bottomRight.first, bottomLeft.second, bottomRight.second), 0.5f, 0.5f);
	normal = glm::normalize(glm::cross(bottomRight.first - pos, bottomLeft.first - pos));
	posNorVec[5] = std::make_pair(pos, normal);

	return posNorVec;
}
