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

#include "Components/AIComponent.hpp"
#include "Components/AnimationComponent.hpp"
#include "Components/GuiComponent.hpp"
#include "Components/PhysicsComponent.hpp"
#include "Components/RenderComponent.hpp"
#include "Components/TextComponent.hpp"
#include "Components/UpdateComponent.hpp"

//Component Managers

#include "Components/AIComponentManager.hpp"
#include "Components/AnimationComponentManager.hpp"
#include "Components/GuiComponentManager.hpp"
#include "Components/PhysicsComponentManager.hpp"
#include "Components/RenderComponentManager.hpp"
#include "Components/UpdateComponentManager.hpp"

//Other stuff

#include "Engine.hpp"
#include "DisplayEngine.hpp"
#include "Camera.hpp"
