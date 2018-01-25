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
#include "GlRenderingEngine.hpp"
#include "ExtraMath.hpp"

Engine::Engine(const EngineConfig& config) :
	config(config) {

	switch(config.renderer) {
		case Renderer::OPEN_GL: renderer.reset(new GlRenderingEngine()); break;
		default: throw std::runtime_error("Incomplete switch in Engine::Engine()");
	}
}

Engine::~Engine() {

}

void Engine::run(GameInterface& game) {
	//Initialize renderer
	renderer->init(config.windowWidth, config.windowHeight, config.windowTitle);

	//Pre-loading of a splash screen might go here

	//Load resources
	renderer->loadDefaultShaders(config.shaderPath);

	game.loadTextures(renderer->getTextureLoader());

	//Enter game loop
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
		while(lag >= config.physicsTimestep && loops < 10) {
			//TODO: update
			lag -= config.physicsTimestep;
			loops++;
		}

		//Running very slow - slow == bad!
		if (loops >= 10) {
			//TODO: warning
			lag = 0.0;
		}

		//Render the game.
		renderer->render((float)lag / config.physicsTimestep);
	}

	//Clean up resources, exit game
}

void Engine::exit() {
	stopped = true;
}

bool Engine::shouldExit() {
	return stopped || renderer->windowClosed();
}
