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

#include <unordered_map>
#include <unordered_set>
#include <string>

#include "ComponentManager.hpp"
#include "RenderComponent.hpp"

class RenderComponentManager : public ComponentManager {
public:
	/**
	 * Constructor, sets name.
	 */
	RenderComponentManager() : ComponentManager(RENDER_COMPONENT_NAME) {}

	/**
	 * RenderComponents don't update.
	 */
	void update() {}

	/**
	 * Called by the renderer to get all render components
	 * @return A list of all render components (needs casting).
	 */
	std::unordered_map<std::string, std::unordered_set<std::shared_ptr<RenderComponent>>>& getComponentShaderMap() { return renderComponents; }

	/**
	 * Removes and readds the component to the render component map.
	 * @param renderComp The component to reload.
	 */
	void reloadComponent(std::shared_ptr<RenderComponent> renderComp, std::string oldShader);

private:
	//Sorts all RenderComponents by their shader for less context switching.
	std::unordered_map<std::string, std::unordered_set<std::shared_ptr<RenderComponent>>> renderComponents;

	/**
	 * Adds the component to one of the internal lists based on its model.
	 * @param comp The component to be added.
	 */
	void onComponentAdd(std::shared_ptr<Component> comp);

	/**
	 * Undoes onComponentAdd.
	 * @param comp The component to un-add.
	 */
	void onComponentRemove(std::shared_ptr<Component> comp);
};
