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

#include <tbb/parallel_for.h>

#include "Engine.hpp"
#include "GameInterface.hpp"
#include "ExtraMath.hpp"
#include "Renderer/RenderingEngine.hpp"

#ifdef USE_OPENGL
#	include "Renderer/Opengl/GlRenderingEngine.hpp"
#	include "Renderer/Opengl/PhysDebRenderingEngine.hpp"
#endif

#ifdef USE_VULKAN
#	include "Renderer/Vulkan/VkRenderingEngine.hpp"
#endif

Engine* Engine::instance = nullptr;

Engine::Engine(const EngineConfig& config) :
	config(config),
	logger(config.generalLog),
	display(),
	modelManager(config.modelLog),
	modelLoader(config.modelLog, modelManager),
	fontManager(modelManager) {

	if (instance) {
		throw std::runtime_error("Engine already initialized!");
	}

	instance = this;

	switch(config.renderer.renderType) {
		case Renderer::OPEN_GL:
#ifdef USE_OPENGL
			ENGINE_LOG_INFO(logger, "Using OpenGL renderer.");
			renderer.reset(new GlRenderingEngine(display, config.rendererLog));
			break;
#else
			ENGINE_LOG_FATAL(logger, "Attempt to use OpenGL rendering engine when OpenGL isn't enabled!");
			throw std::runtime_error("OpenGL rendering engines aren't enabled!");
#endif
		case Renderer::VULKAN:
#ifdef USE_VULKAN
			ENGINE_LOG_INFO(logger, "Using Vulkan renderer.");
			renderer.reset(new VkRenderingEngine(display, config.rendererLog));
			break;
#else
			ENGINE_LOG_FATAL(logger, "Attempt to use vulkan rendering engine when vulkan isn't enabled!");
			throw std::runtime_error("Vulkan rendering engine isn't enabled!");
#endif
		case Renderer::OPEN_GL_PHYSICS_DEBUG:
#ifdef USE_OPENGL
			ENGINE_LOG_INFO(logger, "Using OpenGL renderer, with physics debugging enabled.");
			renderer.reset(new PhysDebRenderingEngine(display, config.rendererLog));
			break;
#else
			ENGINE_LOG_FATAL(logger, "Attempt to use OpenGL-based rendering engine when OpenGL isn't enabled!");
			throw std::runtime_error("OpenGL rendering engines aren't enabled!");
#endif
		default:
			ENGINE_LOG_FATAL(logger, "Unknown renderer requested!");
			throw std::runtime_error("Incomplete switch in Engine::Engine()");
	}

	display.setRenderer(renderer);
	modelManager.setMemoryManager(renderer->getMemoryManager());
}

Engine::~Engine() {
	instance = nullptr;
}

void Engine::run(GameInterface& game) {
	ENGINE_LOG_INFO(logger, "Initializing engine...");

	//Initialize renderer
	ENGINE_LOG_INFO(logger, "Initializing renderer...");

	renderer->init();

	ENGINE_LOG_INFO(logger, "Initializing renderer objects...");

	game.createRenderObjects(renderer->getRenderInitializer());
	renderer->getMemoryManager()->uniformBufferInit();
	ENGINE_LOG_INFO(logger, "Renderer initialization complete.");

	//Pre-loading of a splash screen might go here

	//Load resources
	ENGINE_LOG_INFO(logger, "Beginning resource loading...");

	game.loadShaders(renderer->getShaderLoader());
	ENGINE_LOG_INFO(logger, "Finshed loading shaders.");

	game.loadTextures(renderer->getTextureLoader());
	ENGINE_LOG_INFO(logger, "Finished loading textures.");

	renderer->getMemoryManager()->initializeDescriptors();
	ENGINE_LOG_DEBUG(logger, "Initialized renderer descriptor sets.");

	game.loadModels(modelLoader);
	ENGINE_LOG_INFO(logger, "Finished loading models.");

	game.loadScreens(display);
	ENGINE_LOG_INFO(logger, "Finished loading screens.");

	renderer->finishLoad();
	ENGINE_LOG_INFO(logger, "Load complete.");

	//Enter game loop
	ENGINE_LOG_INFO(logger, "Starting game...");

	double currentTime = ExMath::getTimeMillis();
	double lag = 0.0;
	double lastReportTime = currentTime;
	double totalFrameTime = 0.0;
	size_t numFrames = 0;

	while (!shouldExit()) {
		double frameStart = ExMath::getTimeMillis();

		//Poll for window / input events
		renderer->getWindowInterface().pollEvents();

		//Calculate time since last frame, capping at 100 milliseconds.
		double newTime = ExMath::getTimeMillis();
		double frameTime = newTime - currentTime;

		if (frameTime > 100.0) {
			frameTime = 100.0;
		}

		currentTime = newTime;
		lag += frameTime;

		//Catch up to the current time, but don't go into a catch-up death spiral.
		uint32_t loops = 0;
		while(lag >= config.timestep && loops < 10) {
			display.update();
			lag -= config.timestep;
			loops++;
		}

		//Running very slow - slow == bad!
		//TODO: This doesn't seem to work quite right. Needs testing later.
		if (loops >= 10) {
			ENGINE_LOG_WARN(logger, "Runnning " + std::to_string(lag) + "ms behind.");
			lag = 0.0;
		}

		//Render the game.
		display.render((float)lag / config.timestep);

		double frameEnd = ExMath::getTimeMillis();

		totalFrameTime += frameEnd - frameStart;
		numFrames++;

		if (frameEnd - lastReportTime > config.frameReportFrequency) {
			double averageFrames = totalFrameTime / numFrames;

			ENGINE_LOG_DEBUG(logger,
				std::to_string(numFrames) + " frames completed in " +
				std::to_string(totalFrameTime) + "ms. Average frame time: " + std::to_string(averageFrames) + "ms - " +
				std::to_string(numFrames / (totalFrameTime / 1000)) + " fps");

			totalFrameTime = 0.0;
			numFrames = 0;
			lastReportTime = frameEnd;
		}
	}

	//Clean up resources, exit game
	ENGINE_LOG_INFO(logger, "Exit called, shutting down.");

	//Prevent segmentation faults.
	display.clear();
}

const WindowSystemInterface& Engine::getWindowInterface() const {
	return renderer->getWindowInterface();
}

void Engine::parallelFor(size_t begin, size_t end, const std::function<void(size_t)>& func, size_t grainSize) {
	if (grainSize == 0) {
		tbb::parallel_for(begin, end, func);
	}
	else {
		//Lambdas of lambdas of lambdas...
		tbb::parallel_for(tbb::blocked_range<size_t>(begin, end, grainSize), [&func](const tbb::blocked_range<size_t>& r) {
			for (size_t i = r.begin(); i < r.end(); i++) {
				func(i);
			}
		});
	}
}

bool Engine::shouldExit() {
	return display.shouldExit() || renderer->getWindowInterface().windowClosed();
}
