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

#include "Component.hpp"
#include "Screen.hpp"

class UpdateManager;

class UpdateComponent : public Component {
public:
	enum class UpdateState {
		INACTIVE,
		ACTIVE,
		SLEEPING,
	};

	/**
	 * Required from component.
	 * @return The component's name.
	 */
	static const std::string getName() { return UPDATE_COMPONENT_NAME; }

	//If the component is sleeping, the time to wake it up at. Do not change
	//this without a good reason, such as sleeping again in onWake - the sleep queue in the
	//manager makes the assumption that this doesn't change while the component
	//is in the sleep state.
	size_t wakeTime;
	//Current state of the component. Don't change this outside of onWake, or the
	//component manager will get confused.
	UpdateState state;

	/**
	 * Creates an update component.
	 * @param startingState The state the component will start in.
	 * @param startingTime The amount of time the component will start sleeping for
	 *     when it is added.
	 * @param concurrent Whether the component can update asynchronously.
	 */
	UpdateComponent(UpdateState startingState = UpdateState::ACTIVE, size_t startingTime = 0, bool concurrent = false) :
		wakeTime(startingTime),
		state(startingState),
		manager(nullptr),
		concurrent(concurrent) {}

	virtual ~UpdateComponent() {}

	/**
	 * Does an update. This will only be called concurrently if isConcurrent
	 * returns true. This will never be called if state is INACTIVE or SLEEPING.
	 * @param screen The screen this component is a part of.
	 */
	virtual void update(Screen* screen) {}

	/**
	 * Called when the component's sleep timer runs out. When this function returns,
	 * the component will be moved into the list that corresponds with its currently
	 * set state, and, if it is still SLEEPING, it will sleep for wakeTime more ticks.
	 */
	virtual void onWake() { state = UpdateState::ACTIVE; }

	/**
	 * These are convenience functions to transition the component to the respective state.
	 * @param time The amount of time to sleep for.
	 */
	void activate();
	void deactivate();
	void sleep(size_t time);

	/**
	 * Returns whether the component can be updated asynchronously to other
	 * concurrent components.
	 * @return Whether the component is threadsafe.
	 */
	bool isConcurrent() const { return concurrent; }

	/**
	 * Sets the component's current manager. Only called from UpdateComponentManager.
	 * @param newManager The new manager of the component.
	 */
	void setManager(UpdateManager* newManager) { manager = newManager; }

protected:
	//The parent manager. Will be set once the component is added to the screen.
	UpdateManager* manager;

private:
	//Whether the component can be updated concurrently.
	bool concurrent;
};
