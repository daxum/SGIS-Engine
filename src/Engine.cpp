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
#include "GlRenderer/GlRenderingEngine.hpp"
#include "VkRenderer/VkRenderingEngine.hpp"
#include "ExtraMath.hpp"
#include "LinearMath/btThreads.h"

Engine* Engine::instance = nullptr;

Engine::Engine(const EngineConfig& config) :
	config(config),
	logger(config.generalLog.type, config.generalLog.mask, config.generalLog.outputFile),
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
			logger.info("Using OpenGL renderer.");
			renderer.reset(new GlRenderingEngine(display, config.rendererLog, config.loaderLog));
			break;
		case Renderer::VULKAN:
			logger.info("Using Vulkan renderer.");
			renderer.reset(new VkRenderingEngine(display, config.rendererLog, config.loaderLog));
			break;
		default:
			logger.fatal("Unknown renderer requested!");
			throw std::runtime_error("Incomplete switch in Engine::Engine()");
	}

	display.setRenderer(renderer);
	modelManager.setMemoryManager(renderer->getMemoryManager());
}

Engine::~Engine() {
	instance = nullptr;
}

void Engine::run(GameInterface& game) {
	logger.info("Initializing engine...");

	//Initialize renderer
	logger.info("Initializing renderer...");

	renderer->init();
	logger.info("Renderer initialization complete.");

	//Pre-loading of a splash screen might go here

	//Load resources
	logger.info("Beginning resource loading...");

	game.loadShaders(renderer->getShaderLoader());
	logger.info("Finshed loading shaders.");

	game.loadTextures(renderer->getTextureLoader());
	logger.info("Finished loading textures.");

	game.loadModels(modelLoader);
	logger.info("Finished loading models.");

	game.loadScreens(display);
	logger.info("Finished loading screens.");

	renderer->finishLoad();
	logger.info("Load complete.");

	//Enter game loop
	logger.info("Starting game...");

	double currentTime = ExMath::getTimeMillis();
	double lag = 0.0;

	while (!shouldExit()) {
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
			logger.warn("Runnning " + std::to_string(lag) + "ms behind.");
			lag = 0.0;
		}

		//Render the game.
		display.render((float)lag / config.timestep);
	}

	//Clean up resources, exit game
	logger.info("Exit called, shutting down.");

	//Prevent segmentation faults.
	display.clear();
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
