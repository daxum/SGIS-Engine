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
	//Sorts the RenderComponents by buffer, then shader, then model.
	typedef std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<const Model*, std::unordered_set<const RenderComponent*>>>> RenderPassList;

	/**
	 * Constructor, sets name.
	 */
	RenderComponentManager() : ComponentManager(RENDER_COMPONENT_NAME) {}

	/**
	 * RenderComponents don't update.
	 */
	void update() override {}

	/**
	 * Gets a sorted list of all render components (by buffer, then shader, then model).
	 * @return A sorted list of render components.
	 */
	const RenderPassList& getComponentList() const { return renderComponents; }

	/**
	 * Returns an unsorted set of all render components.
	 * @return A set of render components.
	 */
	const std::unordered_set<const RenderComponent*>& getComponentSet() { return renderComponentSet; }

	/**
	 * Removes and readds the component to the render component list.
	 * @param renderComp The component to reload.
	 * @param oldModel The model the component previously used.
	 */
	void reloadComponent(const RenderComponent* renderComp, std::shared_ptr<const ModelRef> oldModel);

private:
	//Sorts all RenderComponents by their shader for less context switching.
	RenderPassList renderComponents;
	//A set of all RenderComponents, to avoid unneccessary casting.
	std::unordered_set<const RenderComponent*> renderComponentSet;

	/**
	 * Adds the component to one of the internal lists based on its model.
	 * @param comp The component to be added.
	 */
	void onComponentAdd(std::shared_ptr<Component> comp) override;

	/**
	 * Undoes onComponentAdd.
	 * @param comp The component to un-add.
	 */
	void onComponentRemove(std::shared_ptr<Component> comp) override;

	/**
	 * Removes a render component from the manager, and clears its model
	 * if it is not used by anything else.
	 * @param comp The component to remove.
	 * @param oldModel The old model of the render component, or just the
	 *     model if the component is being removed completely.
	 */
	void removeComponent(const RenderComponent* comp, std::shared_ptr<const ModelRef> oldModel);

	/**
	 * Helper function to get the set a component belongs in.
	 * @param model The model of the render component to fetch the set for.
	 * @return The set the component belongs in.
	 */
	std::unordered_set<const RenderComponent*>& getComponentSet(std::shared_ptr<const ModelRef> model);
};
