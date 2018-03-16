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

#include <glm/gtc/matrix_transform.hpp>

#include "EngineConfig.hpp"
#include "DisplayEngine.hpp"
#include "GlfwKeyTranslator.hpp"
#include "GlRenderingEngine.hpp"
#include "GlTextureLoader.hpp"
#include "GlShaderLoader.hpp"
#include "RenderComponentManager.hpp"
#include "RenderComponent.hpp"

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
	//TODO: remove once resizing is figured out
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

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
	/*
	//Other callbacks - might be moved elsewhere to accommodate engine
	glfwSetScrollCallback(window, mouseScroll);
	glfwSetCursorPosCallback(window, cursorMove);
	glfwSetMouseButtonCallback(window, mouseClick);
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

void GlRenderingEngine::loadDefaultShaders(std::string path) {
	shaderLoader->loadShader("basic", path + "glsl/basicShader.vert", path + "glsl/basicShader.frag", nullptr);
}

void GlRenderingEngine::render(ScreenRenderData& data, float partialTicks) {
	//Don't render without a render component.
	if (!data.componentManager) {
		return;
	}

	//Use basic shader for everything for now
	std::shared_ptr<GlShader> shader = shaderMap.at("basic");
	shader->use();

	shader->setUniformMat4("projection", projection);

	MatrixStack matStack;

	matStack.multiply(data.camera.getView());

	//All models use the static buffer at this time
	memoryManager.bindBuffer(MeshType::STATIC);

	//Render all objects
	for (std::shared_ptr<Component> object : data.componentManager->getRenderComponents()) {
		renderObject(matStack, shader, std::static_pointer_cast<RenderComponent>(object));
	}
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

void GlRenderingEngine::setProjection(int width, int height) {
	projection = glm::perspective(PI / 4.0f, (float)width / height, 0.1f, 100000.0f);
}

void GlRenderingEngine::setViewport(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	renderer->setProjection(width, height);
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
		default: nativeAction = KeyAction::UNKNOWN; break;
	}

	display->onKeyAction(GLFWKeyTranslator::translate(key), nativeAction);
}

void GlRenderingEngine::renderObject(MatrixStack& matStack, std::shared_ptr<GlShader> shader, std::shared_ptr<RenderComponent> data) {
	matStack.push();

	matStack.translate(data->getTranslation());
	matStack.rotate(data->getRotation());
	matStack.scale(data->getScale());

	shader->setUniformMat4("modelView", matStack.top());
	shader->setUniformVec3("color", data->getColor());

	Model& model = modelManager.getModel(data->getModel());

	glBindTexture(GL_TEXTURE_2D, textureMap.at(model.texture));

	glDrawElements(GL_TRIANGLES, model.mesh.indexCount, GL_UNSIGNED_INT, (void*) (uintptr_t)model.mesh.indexStart);

	matStack.pop();
}
