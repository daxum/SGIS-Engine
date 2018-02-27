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

#include <string>
#include <memory>
#include <glm/glm.hpp>

#include "RenderData.hpp"
#include "AxisAlignedBB.hpp"

//Represents a map in a world, used mostly for collision detection.
class Map {
public:
	/**
	 * Destructor
	 */
	virtual ~Map() {}

	/**
	 * Returns the height at the given position. Might need tweaking later
	 * for overhangs/caves, but good enough for now.
	 */
	virtual float getHeight(glm::vec3 pos) = 0;

	/**
	 * Returns the outer edge of the map, which only certain things can go past.
	 * @return The border of the map.
	 */
	virtual const AxisAlignedBB& getBorder() = 0;

	/**
	 * Returns the render data for this map. Might be expanded to a vector
	 * later.
	 * @return A pointer to this map's rendering data.
	 */
	virtual std::shared_ptr<RenderData> getRenderData() = 0;
};
