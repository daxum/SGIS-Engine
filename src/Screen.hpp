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

class Screen {
public:
	/**
	 * Constructor
	 */
	Screen();

	/**
	 * Virtual destructor
	 */
	virtual ~Screen() {}

	/**
	 * Updates the screen. This can mean updating a game world, doing animations in a
	 * gui, or nothing at all. This will be called from the bottom of the active screen stack up.
	 */
	virtual void update() = 0;

	/**
	 * Currently not implemented, but this will eventually return all the information required
	 * to render the screen (models for objects, post-processing steps, etc).
	 */
	virtual void getRenderData() {};
};
