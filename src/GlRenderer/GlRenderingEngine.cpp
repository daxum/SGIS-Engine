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
#include "GlRenderInitializer.hpp"

GlRenderingEngine::GlRenderingEngine(DisplayEngine& display, const LogConfig& rendererLog) :
	RenderingEngine(std::make_shared<GlTextureLoader>(textureMap),
					std::make_shared<GlShaderLoader>(&memoryManager, shaderMap),
					std::make_shared<GlRenderInitializer>(memoryManager),
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

	int loadStatus = ogl_LoadFunctions();

	if (loadStatus != 1) {
		throw new std::runtime_error("OpenGl function loading failed");
	}

	ENGINE_LOG_INFO(logger, "Loaded all OpenGL functions.");

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

void GlRenderingEngine::present() {
	glfwSwapBuffers(interface.getWindow());
	glClear(GL_COLOR_BUFFER_BIT);
}

void GlRenderingEngine::setViewport(int width, int height) {
	glViewport(0, 0, width, height);
}

void GlRenderingEngine::renderObjects(const ConcurrentRenderComponentSet& objects, RenderComponentManager::RenderPassList sortedObjects, const Camera* camera, const ScreenState* state) {
	//Opaque objects
	renderTransparencyPass(RenderPass::OPAQUE, objects, sortedObjects, camera, state);

	//Transparent objects
	renderTransparencyPass(RenderPass::TRANSPARENT, objects, sortedObjects, camera, state);

	//Translucent objects
	renderTransparencyPass(RenderPass::TRANSLUCENT, objects, sortedObjects, camera, state);

	glBindVertexArray(0);

	//Clear depth and stencil for next screen
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void GlRenderingEngine::renderTransparencyPass(RenderPass pass, const ConcurrentRenderComponentSet& visibleObjects, const RenderComponentManager::RenderPassList& objects, const Camera* camera, const ScreenState* state) {
	std::string currentBuffer = "";
	std::string currentShader = "";
	bool enableBlend = pass == RenderPass::TRANSLUCENT;
	bool blendOn = false;

	for (const auto& shaderObjectMap : objects) {
		const std::string& buffer = shaderObjectMap.first;

		for (const auto& modelMap : shaderObjectMap.second) {
			const std::string& shaderName = modelMap.first;
			const std::shared_ptr<GlShader> shader = shaderMap.at(shaderName);

			//Skip these objects if their shader isn't in the current render pass
			if (shader->renderPass != pass) {
				continue;
			}

			for (const auto& objectSet : modelMap.second) {
				const Model* model = objectSet.first;
				bool modelSetBound = false;

				for (const RenderComponent* comp : objectSet.second) {
					if (visibleObjects.count(comp)) {
						//Set shader / buffer / blend if needed
						if (currentShader != shaderName) {
							glUseProgram(shader->id);

							//Set screen uniforms for the shader if present
							if (shader->screenSet != "") {
								setPerScreenUniforms(shader.get(), memoryManager.getUniformSet(shader->screenSet), state, camera);
							}

							currentShader = shaderName;
						}

						if (currentBuffer != buffer) {
							memoryManager.bindBuffer(buffer);
							currentBuffer = buffer;
						}

						if (enableBlend && !blendOn) {
							glEnable(GL_BLEND);
							blendOn = true;
						}

						//Set per-model uniforms if not done already
						if (!modelSetBound) {
							setPerModelUniforms(shader.get(), memoryManager.getUniformSet(model->uniformSet), model);
							modelSetBound = true;
						}

						//Set per-object uniforms if needed
						if (shader->objectSet != "") {
							setPerObjectUniforms(shader.get(), memoryManager.getUniformSet(shader->objectSet).uniforms, comp, camera);
						}

						//Set push constants - this is currently exactly the same as the per-object uniforms,
						//will change if uniform buffers are implemented
						setPerObjectUniforms(shader.get(), shader->pushConstants, comp, camera);

						const GlMeshRenderData& renderData = memoryManager.getMeshData(comp->getModel()->getModel().mesh);

						glDrawElements(GL_TRIANGLES, renderData.indexCount, GL_UNSIGNED_INT, (void*) renderData.indexStart);
					}
				}
			}
		}
	}

	if (blendOn) {
		glDisable(GL_BLEND);
	}
}

void GlRenderingEngine::setPerScreenUniforms(const GlShader* shader, const UniformSet& set, const ScreenState* state, const Camera* camera) {
	for (const UniformDescription& uniform : set.uniforms) {
		glm::mat4 tempMat;
		const void* value = nullptr;

		switch (uniform.provider) {
			case UniformProviderType::CAMERA_PROJECTION: tempMat = camera->getProjection(); value = &tempMat; break;
			case UniformProviderType::CAMERA_VIEW: tempMat = camera->getView(); value = &tempMat; break;
			case UniformProviderType::SCREEN_STATE: value = state->getRenderValue(uniform.name); break;
			default: throw std::runtime_error("Invalid provider type for screen uniform set!");
		}

		setUniformValue(shader, uniform.type, uniform.name, value);
	}
}

void GlRenderingEngine::setPerModelUniforms(const GlShader* shader, const UniformSet& set, const Model* model) {
	GLuint nextTextureIndex = 0;

	for (const UniformDescription& uniform : set.uniforms) {
		if (uniform.provider != UniformProviderType::MATERIAL) {
			throw std::runtime_error("Invalid model uniform provider type!");
		}

		//Values, move to buffer later if possible
		if (!isSampler(uniform.type)) {
			switch (uniform.type) {
				case UniformType::FLOAT: {
					float temp = model->uniforms.getFloat(uniform.name);
					setUniformValue(shader, uniform.type, uniform.name, &temp);
				} break;
				case UniformType::VEC2: {
					glm::vec2 temp = model->uniforms.getVec2(uniform.name);
					setUniformValue(shader, uniform.type, uniform.name, &temp);
				} break;
				case UniformType::VEC3: {
					glm::vec3 temp = model->uniforms.getVec3(uniform.name);
					setUniformValue(shader, uniform.type, uniform.name, &temp);
				} break;
				case UniformType::VEC4: {
					glm::vec4 temp = model->uniforms.getVec4(uniform.name);
					setUniformValue(shader, uniform.type, uniform.name, &temp);
				} break;
				case UniformType::MAT3: {
					glm::mat3 temp = model->uniforms.getMat3(uniform.name);
					setUniformValue(shader, uniform.type, uniform.name, &temp);
				} break;
				case UniformType::MAT4: {
					glm::mat4 temp = model->uniforms.getMat4(uniform.name);
					setUniformValue(shader, uniform.type, uniform.name, &temp);
				} break;
				default: throw std::runtime_error("Invalid uniform type in per-model switch!");
			}
		}
		//Images
		else {
			glActiveTexture(GL_TEXTURE0 + nextTextureIndex);
			const GlTextureData& data = textureMap.at(model->textures.at(nextTextureIndex));

			switch (data.type) {
				case TextureType::TEX_2D: glBindTexture(GL_TEXTURE_2D, data.id); break;
				case TextureType::CUBEMAP: glBindTexture(GL_TEXTURE_CUBE_MAP, data.id); break;
				default: throw std::runtime_error("Missing texture type!");
			}

			nextTextureIndex++;
		}
	}
}

void GlRenderingEngine::setPerObjectUniforms(const GlShader* shader, const std::vector<UniformDescription>& set, const RenderComponent* comp, const Camera* camera) {
	for (const UniformDescription& uniform : set) {
		glm::mat4 tempMat;
		const void* value = nullptr;

		switch (uniform.provider) {
			case UniformProviderType::OBJECT_MODEL_VIEW: tempMat = camera->getView() * comp->getTransform(); value = &tempMat; break;
			case UniformProviderType::OBJECT_TRANSFORM: tempMat = comp->getTransform(); value = &tempMat; break;
			case UniformProviderType::OBJECT_STATE: value = comp->getParentState()->getRenderValue(uniform.name); break;
			default: throw std::runtime_error("Invalid provider type for object uniform set!");
		}

		setUniformValue(shader, uniform.type, uniform.name, value);
	}
}
