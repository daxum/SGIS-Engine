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

#include "GameInterface.hpp"
#include "DisplayEngine.hpp"
#include "EngineConfig.hpp"
#include "ModelManager.hpp"
#include "Logger.hpp"
#include "FontManager.hpp"

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
	const std::shared_ptr<const RenderingEngine> getRenderer() const { return renderer; }

	/**
	 * Gets the engine's font manager. Not threadsafe, should only be called from
	 * TextComponent / TextureLoader.
	 * @return The font manager from this engine.
	 */
	FontManager& getFontManager() { return fontManager; }

	/**
	 * Currently only for PhysicsComponentManager.
	 */
	btITaskScheduler* getTaskScheduler() { return scheduler; }

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

	//For threading, probably will change later
	btITaskScheduler* scheduler;

	/**
	 * Indicates if the engine should stop, used to exit the main loop.
	 * @return Whether the engine should stop.
	 */
	bool shouldExit();
};
