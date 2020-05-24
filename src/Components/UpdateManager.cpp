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

#include <algorithm>

#include "UpdateManager.hpp"
#include "Engine.hpp"

void UpdateManager::update() {
	//Sequential updates
	for (UpdateComponent* comp : sequentialComps) {
		comp->update(screen);
	}

	//Concurrent updates
	Engine::parallelFor(0, concurrentComps.size(), [&](size_t val) { concurrentComps.at(val)->update(screen); });

	//Wake sleeping if needed
	for (auto it = sleepingComps.begin(); it != sleepingComps.end(); ) {
		UpdateComponent* comp = *it;

		if (comp->wakeTime <= currentTick) {
			it = sleepingComps.erase(it);
			comp->onWake();
			comp->wakeTime += currentTick + 1;
			addInternal(comp);
		}
		else {
			break;
		}
	}

	//Increment time
	currentTick++;
}

void UpdateManager::moveToState(UpdateComponent* comp, UpdateComponent::UpdateState state, size_t time) {
	removeInternal(comp);
	comp->state = state;
	comp->wakeTime = currentTick + time;
	addInternal(comp);
}

void UpdateManager::addInternal(UpdateComponent* comp) {
	switch (comp->state) {
		//Inactive, don't add to any list
		case UpdateComponent::UpdateState::INACTIVE: break;
		//Active, add to either sequential or concurrent per-tick update list
		case UpdateComponent::UpdateState::ACTIVE: {
			if (comp->isConcurrent()) {
				concurrentComps.push_back(comp);
			}
			else {
				sequentialComps.insert(comp);
			}
		} break;
		//Sleeping, add to sleep queue
		case UpdateComponent::UpdateState::SLEEPING: {
			if (comp->wakeTime < currentTick) {
				//This wouldn't actually result in it sleeping forever, which makes this even worse
				throw std::runtime_error("Invalid attempt to sleep update component forever (wakeTime < currentTick)!");
			}

			sleepingComps.insert(comp);
		} break;
		default: throw std::runtime_error("Invalid update component state!");
	}
}

void UpdateManager::removeInternal(UpdateComponent* comp) {
	switch (comp->state) {
		//Inactive, component wasn't in any lists
		case UpdateComponent::UpdateState::INACTIVE: break;
		//Active, component in either the sequential or concurrent list
		case UpdateComponent::UpdateState::ACTIVE: {
			if (comp->isConcurrent()) {
				auto compLoc = std::find(concurrentComps.begin(), concurrentComps.end(), comp);

				if (compLoc != concurrentComps.end()) {
					*compLoc = concurrentComps.back();
					concurrentComps.pop_back();
				}
				else {
					throw std::runtime_error("Attempt to remove non-present concurrent update component!");
				}
			}
			else {
				sequentialComps.erase(comp);
			}
		} break;
		//Sleeping, component was waiting in the sleep list
		case UpdateComponent::UpdateState::SLEEPING: {
			auto itRange = sleepingComps.equal_range(comp);

			//This is necessary because erase on a multiset removes all elements with the given key.
			//Look into changing the data structure later, probably to a deque + heap.
			for (auto it = itRange.first; it != itRange.second; it++) {
				if (*it == comp) {
					sleepingComps.erase(it);
					break;
				}
			}
		} break;
		default: throw std::runtime_error("Invalid update component state!");
	}
}
