#pragma once

#include <memory>
#include "GameInterface.hpp"
#include "RenderingEngine.hpp"
#include "EngineConfig.hpp"

class Engine {
public:
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
	 * Stops the game engine in preparation for shutting down.
	 * This mostly just exits the main game loop.
	 */
	void exit();

private:
	//The configuration used to create the engine. Non-reference is intentional.
	const EngineConfig config;
	//The rendering engine. The graphics api to be used is set
	//before run is called, during engine configuration.
	std::shared_ptr<RenderingEngine> renderer;
	//Set by the exit function to terminate the main game loop.
	bool stopped;

	/**
	 * Indicates if the engine should stop, used to exit the main loop.
	 * @return Whether the engine should stop.
	 */
	bool shouldExit();
};
