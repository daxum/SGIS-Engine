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

#include <vector>
#include <cstdint>
#include "Screen.hpp"
#include "Object.hpp"

class World : public Screen {
public:
	/**
	 * Creates a world with nothing in it.
	 * @param display The display that stores this world.
	 */
	World(DisplayEngine& display);

	/**
	 * Updates all objects in the world. Will probably be extended to have
	 * some sort of "update hooks" later for pre/post update stuff.
	 */
	void update() {}

	/**
	 * Adds an object to the world. The passed in object should be copy-constructable.
	 * @param object The object to add. The world will use a copy of this object
	 */
	void addObject(std::shared_ptr<Object> object);

	/**
	 * Removes an object from the world.
	 * @param id The id of the object to remove.
	 */
	void removeObject(uint32_t id);

private:
	//Stores everything that is in this world
	std::vector<std::shared_ptr<Object>> objects;
};
