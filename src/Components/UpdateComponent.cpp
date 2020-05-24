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

#include "UpdateManager.hpp"
#include "UpdateComponent.hpp"

//I've given up at this point. The include dependencies have won.
void UpdateComponent::activate() { manager->moveToState(this, UpdateState::ACTIVE); }
void UpdateComponent::deactivate() { manager->moveToState(this, UpdateState::INACTIVE); }
void UpdateComponent::sleep(size_t time) { manager->moveToState(this, UpdateState::SLEEPING, time); }
