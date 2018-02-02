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
#include "TextureLoader.hpp"
#include "ModelLoader.hpp"

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
	 * @param loader The model loader used to load the models.
	 */
	virtual void loadModels(std::shared_ptr<ModelLoader> loader) = 0;

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
