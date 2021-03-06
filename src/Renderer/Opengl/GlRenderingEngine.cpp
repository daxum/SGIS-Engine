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
#include "Display/DisplayEngine.hpp"
#include "GlRenderingEngine.hpp"
#include "GlShaderLoader.hpp"
#include "ExtraMath.hpp"
#include "Display/Camera.hpp"

GlRenderingEngine::GlRenderingEngine(DisplayEngine& display, const LogConfig& rendererLog) :
	RenderingEngine(std::make_shared<GlTextureLoader>(textureMap),
					std::make_shared<GlShaderLoader>(&memoryManager, shaderMap),
					&memoryManager,
					rendererLog),
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

	//Delete shaders

	shaderMap.clear();

	//Clear out memory manager

	memoryManager.deleteObjects();

	//Delete window and terminate glfw

	GLFWwindow* window = interface.getWindow();

	if (window != nullptr) {
		glfwDestroyWindow(window);
	}

	glfwTerminate();

	ENGINE_LOG_INFO(logger, "Destroyed OpenGL rendering engine");
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

	ENGINE_LOG_INFO(logger, "Created window and context");

	//Load OpenGL functions

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		throw new std::runtime_error("OpenGl function loading failed");
	}

	ENGINE_LOG_INFO(logger, "Loaded all OpenGL functions.");

	//Get context info

	GLint major = 0;
	GLint minor = 0;

	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);

	const char* vendor = (const char*) glGetString(GL_VENDOR);
	const char* renderer = (const char*) glGetString(GL_RENDERER);

	ENGINE_LOG_INFO(logger, "OpenGL Info:");
	ENGINE_LOG_INFO(logger, "\tVersion: " + std::to_string(major) + "." + std::to_string(minor));
	ENGINE_LOG_INFO(logger, "\tVendor: " + std::string(vendor));
	ENGINE_LOG_INFO(logger, "\tRenderer: " + std::string(renderer));

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

	ENGINE_LOG_INFO(logger, "OpenGL initialization complete.");
}

void GlRenderingEngine::setViewport(int width, int height) {
	glViewport(0, 0, width, height);
}

void GlRenderingEngine::apiPresent() {
	glfwSwapBuffers(interface.getWindow());
	glClear(GL_COLOR_BUFFER_BIT);
}

void GlRenderingEngine::renderObjects(RenderManager::RenderPassList sortedObjects, const Screen* screen) {
	const Camera* camera = screen->getCamera().get();
	const ScreenState* state = screen->getState().get();

	renderTransparencyPass(RenderPass::OPAQUE, sortedObjects, camera, state);
	renderTransparencyPass(RenderPass::TRANSPARENT, sortedObjects, camera, state);
	renderTransparencyPass(RenderPass::TRANSLUCENT, sortedObjects, camera, state);

	glBindVertexArray(0);

	//Clear depth and stencil for next screen
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void GlRenderingEngine::renderTransparencyPass(RenderPass pass, const RenderManager::RenderPassList& objects, const Camera* camera, const ScreenState* state) {
	bool enableBlend = pass == RenderPass::TRANSLUCENT;
	bool blendOn = false;

	for (const auto& shaderObjectMap : objects) {
		for (const auto& modelMap : shaderObjectMap.second) {
			const std::string& shaderName = modelMap.first;
			const std::shared_ptr<GlShader> shader = shaderMap.at(shaderName);
			bool shaderBound = false;
			bool screenSetBound = false;

			//Skip these objects if their shader isn't in the current render pass
			if (shader->renderPass != pass) {
				continue;
			}

			for (const auto& objectSet : modelMap.second) {
				const Material* material = objectSet.first;
				bool materialSetBound = false;

				for (const RenderComponent* comp : objectSet.second) {
					if (comp->isVisible()) {
						//Set shader / buffer / blend if needed
						if (!shaderBound) {
							glUseProgram(shader->id);
							glBindVertexArray(shader->vao);

							const Mesh* mesh = comp->getModel().mesh;
							const VertexFormat* format = mesh->getFormat();
							const Mesh::BufferInfo& buffers = mesh->getBufferInfo();

							glBindVertexBuffer(0, ((GlBuffer*)buffers.vertex)->getBufferId(), 0, format->getVertexSize());
							glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ((GlBuffer*)buffers.index)->getBufferId());

							shaderBound = true;
						}

						if (enableBlend && !blendOn) {
							glEnable(GL_BLEND);
							blendOn = true;
						}

						size_t nextUniformIndex = 0;

						//Set screen set
						if (!shader->screenSet.empty()) {
							if (!screenSetBound) {
								Std140Aligner& screenAligner = memoryManager.getDescriptorAligner(shader->screenSet);

								setPerScreenUniforms(memoryManager.getUniformSet(shader->screenSet), screenAligner, state, camera);
								GlBuffer* uniBuf = (GlBuffer*) memoryManager.getUniformBuffer(RendererMemoryManager::UniformBufferType::SCREEN_OBJECT);
								uintptr_t offset = memoryManager.writePerFrameUniforms(screenAligner, currentFrame);
								uintptr_t size = screenAligner.getData().second;

								glBindBufferRange(GL_UNIFORM_BUFFER, nextUniformIndex, uniBuf->getBufferId(), offset, size);
								screenSetBound = true;
							}

							nextUniformIndex++;
						}

						//Set material set
						if (!materialSetBound) {
							if (material->hasBufferedUniforms) {
								GlBuffer* uniBuf = (GlBuffer*) memoryManager.getUniformBuffer(RendererMemoryManager::UniformBufferType::MATERIAL);
								uintptr_t offset = material->uniformOffset;
								uintptr_t size = material->uniforms.getData().second;

								glBindBufferRange(GL_UNIFORM_BUFFER, nextUniformIndex, uniBuf->getBufferId(), offset, size);
							}

							//Bind textures
							for (size_t i = 0; i < material->textures.size(); i++) {
								glActiveTexture(GL_TEXTURE0 + i);
								const GlTextureData& texData = textureMap.at(material->textures.at(i));
								glBindTexture(texData.type, texData.id);
							}

							materialSetBound = true;
						}

						if (material->hasBufferedUniforms) {
							nextUniformIndex++;
						}

						//Set object set
						if (!shader->objectSet.empty()) {
							Std140Aligner& objectAligner = memoryManager.getDescriptorAligner(shader->objectSet);

							setPerObjectUniforms(memoryManager.getUniformSet(shader->objectSet), objectAligner, comp, camera);
							GlBuffer* uniBuf = (GlBuffer*) memoryManager.getUniformBuffer(RendererMemoryManager::UniformBufferType::SCREEN_OBJECT);
							uintptr_t offset = memoryManager.writePerFrameUniforms(objectAligner, currentFrame);
							uintptr_t size = objectAligner.getData().second;

							glBindBufferRange(GL_UNIFORM_BUFFER, nextUniformIndex, uniBuf->getBufferId(), offset, size);
						}

						setPushConstants(shader.get(), comp, camera);

						const std::tuple<uintptr_t, uint32_t, int32_t> meshInfo = comp->getModel().mesh->getRenderInfo();
						glDrawElementsBaseVertex(GL_TRIANGLES, std::get<1>(meshInfo), GL_UNSIGNED_INT, (void*) (std::get<0>(meshInfo) * sizeof(uint32_t)), std::get<2>(meshInfo));
					}
				}
			}
		}
	}

	if (blendOn) {
		glDisable(GL_BLEND);
	}
}

void GlRenderingEngine::setPushConstants(const GlShader* shader, const RenderComponent* comp, const Camera* camera) {
	for (const UniformDescription& uniform : shader->pushConstants) {
		//Get push constant value
		glm::mat4 tempMat;
		const void* value = nullptr;

		switch (uniform.provider) {
			case UniformProviderType::OBJECT_MODEL_VIEW: tempMat = camera->getView() * comp->getTransform(); value = &tempMat; break;
			case UniformProviderType::OBJECT_TRANSFORM: tempMat = comp->getTransform(); value = &tempMat; break;
			case UniformProviderType::OBJECT_STATE: value = comp->getParentState()->getRenderValue(uniform.name); break;
			default: throw std::runtime_error("Invalid provider type for object uniform set!");
		}

		//Set push constant in shader
		GLuint uniformLoc = shader->getPushLoc(uniform.name);

		switch (uniform.type) {
			case UniformType::FLOAT: glUniform1f(uniformLoc, *(const float*)value); break;
			case UniformType::VEC2: glUniform2fv(uniformLoc, 1, (const float*)value); break;
			case UniformType::VEC3: glUniform3fv(uniformLoc, 1, (const float*)value); break;
			case UniformType::VEC4: glUniform4fv(uniformLoc, 1, (const float*)value); break;
			case UniformType::MAT3: glUniformMatrix3fv(uniformLoc, 1, GL_FALSE, (const float*)value); break;
			case UniformType::MAT4: glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, (const float*)value); break;
			default: throw std::runtime_error("Invalid uniform type for uniform \"" + uniform.name + "\"!");
		}
	}
}
