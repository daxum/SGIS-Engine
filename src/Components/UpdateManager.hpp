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

#include <set>
#include <unordered_set>
#include <vector>

#include "ComponentManager.hpp"
#include "UpdateComponent.hpp"

class UpdateManager : public ComponentManager {
public:
	UpdateManager() : ComponentManager(UPDATE_COMPONENT_NAME), currentTick(0) {}

	/**
	 * Updates all the update components. First does sequential updates,
	 * then concurrent, then wakes all components whose timers are up.
	 */
	void update() override;

	/**
	 * Moves the component to the given state. If the new state is SLEEPING, time gives the
	 * amount of time to sleep for. The component's currently set state and time (before calling
	 * this function) are used for removal from its old state, so don't mess with those.
	 * @param comp The component to move.
	 * @param state The new state for the component.
	 * @param time If the new state is SLEEPING, the amount of time to sleep for.
	 */
	void moveToState(UpdateComponent* comp, UpdateComponent::UpdateState state, size_t time = 0);

private:
	struct ComponentWakeOrder {
		bool operator()(const UpdateComponent* c1, const UpdateComponent* c2) const {
			return c1->wakeTime < c2->wakeTime;
		}
	};

	//Time since this manager was first added to the screen.
	size_t currentTick;

	//Update components that need to be updated sequentially.
	std::unordered_set<UpdateComponent*> sequentialComps;
	//Update components that can be updated in parallel.
	std::vector<UpdateComponent*> concurrentComps;
	//Update components that are waiting for a certain amount of time to pass.
	std::multiset<UpdateComponent*, ComponentWakeOrder> sleepingComps;

	/**
	 * Called immediately after a component is added to the manager's
	 * internal list.
	 * @param comp The component that was added.
	 */
	void onComponentAdd(std::shared_ptr<Component> comp) override {
		UpdateComponent* upComp = std::static_pointer_cast<UpdateComponent>(comp).get();
		upComp->setManager(this);
		//Adjust wake time for if the component starts sleeping - changes it
		//from an amount to an end time.
		upComp->wakeTime += currentTick;
		addInternal(upComp);
	}

	/**
	 * Called immediately after a component is removed from the manager's
	 * internal list.
	 * @param comp The component that was removed.
	 */
	void onComponentRemove(std::shared_ptr<Component> comp) override {
		removeInternal(std::static_pointer_cast<UpdateComponent>(comp).get());
	}

	/**
	 * Adds the component to the proper internal list using its current
	 * state (and wake time, if sleeping), or none at all if it is inactive.
	 * @param comp The component to add.
	 */
	void addInternal(UpdateComponent* comp);

	/**
	 * Removes the component from all internal lists.
	 * @param comp The component to remove.
	 */
	void removeInternal(UpdateComponent* comp);
};
