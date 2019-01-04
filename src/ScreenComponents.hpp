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

/**
 * Include file for all components, component managers, and a few other things typically used
 * with components and managers.
 */

//Screen

#include "Screen.hpp"

//Components

#include "AIComponent.hpp"
#include "AnimationComponent.hpp"
#include "GuiComponent.hpp"
#include "PhysicsComponent.hpp"
#include "RenderComponent.hpp"
#include "TextComponent.hpp"
#include "UpdateComponent.hpp"

//Component Managers

#include "AIComponentManager.hpp"
#include "AnimationComponentManager.hpp"
#include "GuiComponentManager.hpp"
#include "PhysicsComponentManager.hpp"
#include "RenderComponentManager.hpp"
#include "UpdateComponentManager.hpp"

//Other stuff

#include "Engine.hpp"
#include "DisplayEngine.hpp"
#include "Camera.hpp"
