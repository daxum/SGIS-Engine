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
#include "GlShader.hpp"

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

void GlRenderingEngine::renderObjects(const tbb::concurrent_unordered_set<RenderComponent*>& objects, RenderComponentManager::RenderPassList sortedObjects, std::shared_ptr<Camera> camera, std::shared_ptr<ScreenState> state) {
	MatrixStack matStack;
	matStack.multiply(camera->getView());

	//Opaque objects
	renderTransparencyPass(RenderPass::OPAQUE, objects, sortedObjects, matStack, camera, state);

	//Transparent objects
	renderTransparencyPass(RenderPass::TRANSPARENT, objects, sortedObjects, matStack, camera, state);

	//Translucent objects
	renderTransparencyPass(RenderPass::TRANSLUCENT, objects, sortedObjects, matStack, camera, state);

	glBindVertexArray(0);
}

void GlRenderingEngine::renderObject(MatrixStack& matStack, std::shared_ptr<Shader> shader, std::shared_ptr<RenderComponent> data, std::shared_ptr<ScreenState> state) {
	matStack.push();

	shader->setPerObjectUniforms(data, matStack, state);

	const GlMeshRenderData& renderData = memoryManager.getMeshData(data->getModel()->getModel().mesh);
	glDrawElements(GL_TRIANGLES, renderData.indexCount, GL_UNSIGNED_INT, (void*) renderData.indexStart);

	matStack.pop();
}

void GlRenderingEngine::renderTransparencyPass(RenderPass pass, const tbb::concurrent_unordered_set<RenderComponent*>& visibleObjects, const RenderComponentManager::RenderPassList& objects, MatrixStack& matStack, std::shared_ptr<Camera> camera, std::shared_ptr<ScreenState> state) {
	std::string currentBuffer = "";
	std::string currentShader = "";
	bool enableBlend = pass == RenderPass::TRANSLUCENT;
	bool blendOn = false;

	for (const auto& shaderObjectMap : objects) {
		const std::string& buffer = shaderObjectMap.first;

		for (const auto& modelMap : shaderObjectMap.second) {
			const std::string& shader = modelMap.first;
			const std::shared_ptr<Shader> shaderInter = shaderMap.at(shader);
			const std::shared_ptr<GlShader> shaderObj = std::static_pointer_cast<GlShader>(shaderInter->getRenderInterface());

			//Skip these objects if their shader isn't in the current render pass
			if (shaderObj->renderPass != pass) {
				continue;
			}

			for (const auto& objectSet : modelMap.second) {
				for (const std::shared_ptr<RenderComponent> comp : objectSet.second) {
					if (visibleObjects.count(comp.get())) {
						//Set shader / buffer / blend if needed
						if (currentShader != shader) {
							logger.debug("Setting shader to \"" + shader + "\"");
							glUseProgram(shaderObj->id);
						}

						if (currentBuffer != buffer) {
							logger.debug("Setting buffer to \"" + buffer + "\"");
							memoryManager.bindBuffer(buffer);
						}

						if (enableBlend && !blendOn) {
							logger.debug("Enabling blend");
							glEnable(GL_BLEND);
							blendOn = true;
						}

						renderObject(matStack, shaderInter,comp, state);
					}
				}
			}
		}
	}

	if (blendOn) {
		logger.debug("Disabling blend");
		glDisable(GL_BLEND);
	}
}
