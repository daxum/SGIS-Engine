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

#include "Engine.hpp"
#include "GlRenderer/GlRenderingEngine.hpp"
#include "ExtraMath.hpp"

Engine::Engine(const EngineConfig& config) :
	config(config),
	logger(config.generalLog.type, config.generalLog.mask, config.generalLog.outputFile),
	stopped(false) {

	switch(config.renderer) {
		case Renderer::OPEN_GL:
			logger.info("Using OpenGL renderer.");
			renderer.reset(new GlRenderingEngine(config.rendererLog, config.loaderLog));
			break;
		default:
			logger.fatal("Unknown renderer requested!");
			throw std::runtime_error("Incomplete switch in Engine::Engine()");
	}
}

Engine::~Engine() {

}

void Engine::run(GameInterface& game) {
	logger.info("Initializing engine...");

	//Initialize renderer
	logger.info("Initializing renderer...");

	renderer->init(config.windowWidth, config.windowHeight, config.windowTitle, &display);
	logger.info("Renderer initialization complete.");

	//Pre-loading of a splash screen might go here

	//Load resources
	logger.info("Beginning resource loading...");

	renderer->loadDefaultShaders(config.shaderPath);
	logger.info("Finshed loading shaders.");

	game.loadTextures(renderer->getTextureLoader());
	logger.info("Finished loading textures.");

	game.loadModels(renderer->getModelLoader());
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
		renderer->pollEvents();

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
		display.render((float)lag / config.timestep, renderer);
	}

	//Clean up resources, exit game
	logger.info("Exit called, shutting down.");
}

bool Engine::shouldExit() {
	return display.shouldExit() || renderer->windowClosed();
}
