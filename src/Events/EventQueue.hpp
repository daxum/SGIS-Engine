/******************************************************************************
 * SGIS-Engine - the engine for SGIS
 * Copyright (C) 2020
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

#include <deque>
#include <memory>

#include "EventListener.hpp"

class EventQueue : public EventListener {
public:
	/**
	 * Adds the given listener to the listener list, so that
	 * it will be notified of events.
	 * @param listener The listener to add.
	 */
	void addListener(std::shared_ptr<EventListener> listener) {
		listeners.push_back(listener);
	}

	/**
	 * Adds the given listener to the listener list, but with higher
	 * priority than anything else.
	 * @param listener The listener to add.
	 */
	void addListenerFirst(std::shared_ptr<EventListener> listener) {
		listeners.push_front(listener);
	}

	/**
	 * Removes the listener from the listener list.
	 * @param listener The listener to remove.
	 */
	void removeListener(std::shared_ptr<EventListener> listener);

	/**
	 * Called from the event handler when an event happens. The handler is
	 * usually either the top-level display engine or another EventQueue.
	 * Forwards the event to each of the listeners in the order they were added.
	 * @param event The event.
	 * @return Whether to cancel the event - to stop propagating it. For
	 *     EventQueue, if any of its listeners say to cancel, it also cancels.
	 */
	bool onEvent(const std::shared_ptr<const Event> event) override;

	/**
	 * Removes every listener to prepare for destruction.
	 */
	void removeAllListeners() {
		std::deque<std::shared_ptr<EventListener>>().swap(listeners);
	}

private:
	//Listeners for this queue. All events are passed to each listener in order, unless cancelled.
	std::deque<std::shared_ptr<EventListener>> listeners;
};
