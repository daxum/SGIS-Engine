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
#include <memory>
#include <cstdint>

#include "ObjectRenderData.hpp"

//Stores all the rendering data for the screen, sorted for hopefully optimal
//rendering. This includes objects, map stuff, effects, etc.
class ScreenRenderData {
public:
	/**
	 * Adds an object to be rendered.
	 * @param object The new object to be rendered.
	 */
	void addObject(std::shared_ptr<ObjectRenderData> object);

	/**
	 * Removes the object from the rendering list.
	 * @param id The id of the object to be removed.
	 */
	void removeObject(uint32_t id);

	//Just keep everything in a massive list for now.
	std::vector<std::shared_ptr<ObjectRenderData>> objects;
};
