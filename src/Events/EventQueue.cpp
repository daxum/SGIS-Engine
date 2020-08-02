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

#include <algorithm>
#include <stdexcept>

#include "EventQueue.hpp"

void EventQueue::removeListener(std::shared_ptr<EventListener> listener) {
	auto loc = std::find(listeners.begin(), listeners.end(), listener);

	if (loc != listeners.end()) {
		listeners.erase(loc);
	}
	else {
		throw std::runtime_error("Attempt to remove nonexistant event listener");
	}
}

bool EventQueue::onEvent(const std::shared_ptr<const Event> event) {
	for (std::shared_ptr<EventListener> listener : listeners) {
		if (listener->onEvent(event)) {
			return true;
		}
	}

	return false;
}
