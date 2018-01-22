#pragma once

#include <memory>
#include "TextureLoader.hpp"

//The engine uses the game interface for all communications
//with the game, such as for loading resources.
class GameInterface {
public:
	/**
	 * Only present so implementers can delete their stuff
	 */
	virtual ~GameInterface() {}

	/**
	 * Called by the engine to load the game's textures.
	 * All texture loading should be done in this function.
	 * This function may be called asynchronously to other
	 * loading operations - do not do any outside work here.
	 * Only use the loader provided.
	 * @param loader The texture loader used to load the textures.
	 */
	virtual void loadTextures(std::shared_ptr<TextureLoader> loader) = 0;

	/**
	 * Called by the engine to load the game's models.
	 * Like textures, all model loading must be done in
	 * this function.
	 * Not currently implemented.
	 */
	virtual void loadModels() = 0;

	/**
	 * Called by the engine to load all of the game's guis.
	 * These are things like the start screen and options screens.
	 * The starting menu is also defined here (ie. what the engine
	 * opens after startup finishes).
	 * Not currently implemented.
	 */
	virtual void loadMenus() = 0;

	/**
	 * Called by the engine to load the game's map data. This is currently
	 * not implemented, and is never called.
	 */
	virtual void loadMaps() = 0;
};
