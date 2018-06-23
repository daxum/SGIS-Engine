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

#include <vulkan/vulkan.h>

#include "Logger.hpp"

//Handles creation and destruction of all vulkan objects.
class VkObjectHandler {
public:
	/**
	 * Initializes vulkan objects.
	 */
	VkObjectHandler(Logger& logger);

	/**
	 *Destroys all active objects.
	 */
	~VkObjectHandler();

private:
	//The logger
	Logger& logger;

	//Vulkan instance
	VkInstance instance;

	/**
	 * Creates the instance object.
	 */
	void createInstance();
};
