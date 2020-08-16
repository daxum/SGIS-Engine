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

#include "DisplayEngine.hpp"
#include "Renderer/RenderingEngine.hpp"
#include "Screen.hpp"
#include "Camera.hpp"
#include "Engine.hpp"
#include "Input/InputMapSyncEvent.hpp"

DisplayEngine::DisplayEngine() :
	popped(false) {

}

void DisplayEngine::pushScreen(std::shared_ptr<Screen> screen) {
	if (!screenStack.empty()) {
		//Remove all of current screen stack from event queue
		for (std::shared_ptr<Screen> screen : screenStack.back()) {
			events.removeListener(screen->getEventQueue());
		}
	}

	screenStack.emplace_back();
	pushOverlay(screen);
}

void DisplayEngine::popScreen() {
	if (!screenStack.empty()) {
		//Remove all of current screen stack from event queue
		for (std::shared_ptr<Screen> screen : screenStack.back()) {
			events.removeListener(screen->getEventQueue());
		}
	}

	screenStack.pop_back();

	popped = true;

	if (!screenStack.empty() && !screenStack.back().empty()) {
		renderer->getWindowInterface().captureMouse(getTop()->mouseHidden());

		//Put the new top of the screen stack on the event queue
		for (std::shared_ptr<Screen> screen : screenStack.back()) {
			events.addListenerFirst(screen->getEventQueue());
		}

		//Sync input state
		events.onEvent(std::make_shared<InputMapSyncEvent>());
	}
}

void DisplayEngine::pushOverlay(std::shared_ptr<Screen> overlay) {
	if (screenStack.empty()) {
		screenStack.emplace_back();
	}

	screenStack.back().push_back(overlay);
	renderer->getWindowInterface().captureMouse(overlay->mouseHidden());
	events.addListenerFirst(overlay->getEventQueue());

	//Need an initial update here for new screens for GuiManager to update correctly
	overlay->update();

	//Sync input state
	events.onEvent(std::make_shared<InputMapSyncEvent>());
}

void DisplayEngine::popOverlay() {
	events.removeListener(screenStack.back().back()->getEventQueue());
	screenStack.back().pop_back();

	if (!screenStack.back().empty()) {
		renderer->getWindowInterface().captureMouse(getTop()->mouseHidden());

		//Sync input state
		events.onEvent(std::make_shared<InputMapSyncEvent>());
	}
}

std::shared_ptr<Screen> DisplayEngine::getTop() {
	return screenStack.back().back();
}

void DisplayEngine::update() {
	if (screenStack.empty()) {
		return;
	}

	//Update the overlay stack from back to front.
	//This cannot be range-based, because modifying the
	//container you're looping over tends to behave strangely,
	//due to the iterators not getting updated.
	for (size_t i = screenStack.back().size(); i > 0; i--) {
		//If the overlay stack got popped more than once last update, i can become invalid.
		if (i > screenStack.back().size()) {
			i = screenStack.back().size();
		}

		std::shared_ptr<Screen> current = screenStack.back()[i - 1];
		current->update();

		//If the screen stack was popped in the last update, screenStack.top() now points
		//to a different overlay stack, so we should stop updating for this tick.
		if (popped) {
			break;
		}
	}

	popped = false;
}

void DisplayEngine::render(float partialTicks) {
	if (screenStack.empty()) {
		return;
	}

	renderer->beginFrame();

	//Render all screens in the overlay stack from bottom to top.
	for (std::shared_ptr<Screen> screen : screenStack.back()) {
		renderer->render(screen.get());
	}

	renderer->present();
}

void DisplayEngine::setRenderer(std::shared_ptr<RenderingEngine> newRenderer) {
	renderer = newRenderer;
}

bool DisplayEngine::shouldExit() {
	return screenStack.empty();
}

void DisplayEngine::updateProjections() {
	for (std::vector<std::shared_ptr<Screen>>& overlay : screenStack) {
		for (std::shared_ptr<Screen>& screen : overlay) {
			screen->getCamera()->setProjection();
		}
	}
}
