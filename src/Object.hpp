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
#include <string>
#include <cstdint>

class ObjectRenderData;

//An object in a world. Stores rendering, physics, etc.
class Object {
public:
	//A unique id for this object. Unless over 4 billion objects somehow get created.
	const uint32_t id;

	/**
	 * Creates an object.
	 * @param model The name of the model to use when rendering.
	 */
	Object(std::string model);

	/**
	 * Returns model used and similar data.
	 */
	std::shared_ptr<ObjectRenderData> getRenderData() { return renderData; }

private:
	//Has to be pointer or else vector throws a fit.
	std::shared_ptr<ObjectRenderData> renderData;
};
