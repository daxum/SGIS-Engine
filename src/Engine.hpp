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

#pragma once

#include <memory>
#include <functional>

#include "GameInterface.hpp"
#include "DisplayEngine.hpp"
#include "EngineConfig.hpp"
#include "ModelManager.hpp"
#include "Logger.hpp"
#include "FontManager.hpp"
#include "WindowSystemInterface.hpp"

class RenderingEngine;
class btITaskScheduler;

class Engine {
public:
	//Global engine instance. Use sparingly!
	static Engine* instance;

	/**
	 * Constructs an instance of the game engine.
	 * Does not initialize any subsystems, but allows
	 * configuration of the engine before run is called.
	 * @throw runtime_error if creation of one of the subsystems failed.
	 */
	Engine(const EngineConfig& config);

	/**
	 * Just a destructor. It destructs stuff.
	 * If run was never called it probably won't do much.
	 */
	~Engine();

	/**
	 * Starts the game engine. This will create a window,
	 * initialize the renderer, and load resources from the
	 * provided game object, then enter the main game loop.
	 * @param game The game to be run by the engine
	 * @throw runtime_error if initialization of one of the subsytems failed.
	 */
	void run(GameInterface& game);

	/**
	 * Returns the model manager for this engine. Const because it should
	 * only be used for looking up model properties.
	 */
	const ModelManager& getModelManager() const { return modelManager; }

	/**
	 * Gets the configuration the engine was created with.
	 * @return the engine's configuration.
	 */
	const EngineConfig& getConfig() const { return config; }

	/**
	 * Gets a constant version of the current rendering engine.
	 * Used to get things like the projection matrix and window size.
	 * @return The rendering engine.
	 */
	const WindowSystemInterface& getWindowInterface() const { return renderer->getWindowInterface(); }

	/**
	 * Gets the engine's font manager. Not threadsafe, should only be called from
	 * TextComponent / TextureLoader.
	 * @return The font manager from this engine.
	 */
	FontManager& getFontManager() { return fontManager; }

	/**
	 * Execute for loop in parallel.
	 * @param begin The starting value.
	 * @param end The ending value.
	 * @param func The function to run. Will be called with the current index of the loop.
	 * @param grainSize How many invokations of func to do serially per task. Set to zero to determine automatically.
	 */
	void parallelFor(size_t begin, size_t end, const std::function<void(size_t)>& func, size_t grainSize = 0);

private:
	//The configuration used to create the engine. Non-reference is intentional.
	const EngineConfig config;
	//The place the engine logs messages to.
	Logger logger;
	//The display manager. Handles rendering, updating, and input for multiple "screens" (game world, huds, menus, etc) at once.
	//Name seems a bit misleading.
	DisplayEngine display;
	//The rendering engine. The graphics api to be used is set
	//before run is called, during engine configuration.
	std::shared_ptr<RenderingEngine> renderer;
	//Manages all models loaded by the game
	ModelManager modelManager;
	//Manages all loaded fonts
	FontManager fontManager;

	/**
	 * Indicates if the engine should stop, used to exit the main loop.
	 * @return Whether the engine should stop.
	 */
	bool shouldExit();
};
